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
#include "dispowercensus.h"
#include "arkpp.h"
#include "enginearkppclasses.h"

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


// agentDO:actorfeed
/*=============================================================================
	Agent DO: the ACTOR feed of the post-process settings path (agent DE hand-over 1).

	ULocalPlayer::UpdatePostProcessSettings (2013 rva 0x2b08b0) fills m_CurrentArkPpSettings from four feeds and the
	third of them is APlayerController::ModifyPostProcessSettings. In the base it is empty; ADishonoredPlayerController's
	override (0x6adeb0) is where the powers reach the colour grade.

	It does two things. The first time it runs it finds twenty-seven post-process nodes by name and caches them on
	UDisPostProcessManager::m_PpBridge, and the bridge's m_bInitDone bit is the gate on every effect the manager's own
	Tick drives - until this function runs once, no power can show at all. Then, every frame, it folds in the camera's
	post-process targets, the water volume's override, the player's health effects and the dark-vision power.

	DISHONORED(retail): three things this file's comments correct.

	1. Retail does NOT tick the post-process node controllers here (agent DE defect 7 said it did). The controllers'
	   Tick is vtable slot +304 in retail 2013 (+300 in the 2012 build - UObject gained a virtual), and the only call
	   site in either executable is ADishonoredPlayerController::Tick (0x6b6e80, 2012 0x6ef510), which walks the first
	   local player's post-process chain and ticks each node's controller. Scanned for exhaustively: every `mov r32,
	   [reg+304]` and `call [reg+304]` in the whole .text of both builds (build/agentDO_work/vslot2r_304.txt,
	   vslot300.txt). That walk is ported below, which is what makes agent DE's three controllers live.
	2. Retail 2013 backs up the BendTime node's uber parameters inside the init block, right after finding the node;
	   the 2012 build does not. Ported as retail 2013 has it.
	3. Retail 2013's init block is a straight sequence of DisGetArkPpNode/DisGetArkPpNodeMaterial calls with no null
	   test until the end, where every one of the twenty-four pointers that matter is tested at once and m_bInitDone is
	   only raised if all of them resolved. So a chain that is missing one node retries the whole lookup every frame,
	   for ever. Kept exactly as retail has it; the census counts the misses so that state is visible rather than silent.

	DISHONORED(bringup): five of retail's calls are named rather than ported, each with what it needs.
	  ADishonoredPlayerController::UpdateSunBlindingEffect (0x6a6450, 390 bytes) - walks
	    AWorldInfo::m_SunMeshesAndMaterials @600 and pushes a bloom value into each entry's m_SunDynamicMIC through
	    SetScalarParameterValue with a HARDCODED FName index (1300, 0). That index is retail's name table, not ours, and
	    guessing a material parameter name is exactly the unverified change this project keeps paying for. It is the sun
	    glare, not a power.
	  ADishonoredPlayerController::ApplyAdrenalineProcessSettings (0x6a65e0, 367 bytes) - m_fAdrenalineWeight @1780
	    rises and falls against UDisTweaks_PlayerPawn_Combat's adrenaline pair and lands in
	    UDisPostProcessManager::m_AdrenalineParams; it needs ADishonoredPlayerPawn's slow-motion predicate (0x6a21e0)
	    and FDisZoneTracker::IsInZone, both unported.
	  ADishonoredPlayerController::ApplyMusicalOverseerPostProcessSettings (0x6a6970, 691 bytes) - gated on
	    ADishonoredPawn::ArePowersInhibited; it needs a second tweaks class and the heart's proximity curve.
	  UDisPostProcessManager::ApplyKismetPostProcessSettings (0x7ef9b0) and ApplyUIPostProcessSettings (0x7efca0) - the
	    Kismet and UI uber pushes, driven by SetKismetPPParams/SetUIPPParams, which nothing in this tree calls yet.
	  UDisPostProcessManager::Tick (2012 0x857430, 2,548 bytes, called from ADishonoredPlayerPawn::Tick) - the node
	    visibility and material drive for underwater, bend time, blink, adrenaline, plague, the knock-out and the two
	    possession stages. Dark vision does NOT go through it: its node is driven by its own controller, which is why
	    dark vision is what this package can show end to end.
=============================================================================*/

INT GDisPpBridgeInits = 0;
INT GDisPpBridgeNodesFound = 0;
INT GDisPpBridgeNodesMissing = 0;
INT GDisPpControllerTicks = 0;
INT GDisPpModifyCalls = 0;
INT GDisTallboyStiltsSet = 0;
INT GDisTallboyLightsSpawned = 0;

static INT GDisPowerCensus = -1;
static INT GDisDarkVisionForce = -1;
static INT GDisModifyPpOff = -1;

UBOOL DisPowerCensusEnabled()
{
	// DISHONORED(bringup): read on first use. A file-scope static initialiser in a static library runs before WinMain
	// sets GCmdLine and the switch would then always be FALSE (agent CA, PHASE10 rule).
	if( GDisPowerCensus < 0 )
	{
		GDisPowerCensus = ( appStrfind( appCmdLine(), TEXT("-dispowerdbg") ) != NULL ) ? 1 : 0;
	}
	return GDisPowerCensus != 0;
}

UBOOL DisDarkVisionForced()
{
	if( GDisDarkVisionForce < 0 )
	{
		GDisDarkVisionForce = ( appStrfind( appCmdLine(), TEXT("-disdarkvisionpp") ) != NULL ) ? 1 : 0;
	}
	return GDisDarkVisionForce != 0;
}

UBOOL DisModifyPpSuppressed()
{
	if( GDisModifyPpOff < 0 )
	{
		GDisModifyPpOff = ( appStrfind( appCmdLine(), TEXT("-nodismodifypp") ) != NULL ) ? 1 : 0;
	}
	return GDisModifyPpOff != 0;
}

/**
 * DISHONORED(port): 2013 rva 0x6b6e80 (2012 0x6ef510) - and this is the ONLY caller of UArkPpNodeController::Tick in
 * either executable. It walks the first local player's post-process chain and ticks every node's controller with the
 * frame's delta, which is what advances UDisOpacityParameterPpController::m_CurrentTime and
 * UDisDarkVisionPpController::m_EyeLidTime / m_PowerTime. Retail reads the chain off Player, not off GEngine's array,
 * so a controller with no local player ticks nothing.
 *
 * DISHONORED(bringup): ADishonoredPlayerController::TickZoom (0x6b8000 region, 2012 0x6ef5e0) closes retail's body and
 * is not ported: it drives the zoom lens through UDisPostProcessManager::TickZoomLens, which is itself unported.
 */
UBOOL ADishonoredPlayerController::Tick( FLOAT DeltaTime, enum ELevelTick TickType )
{
	const UBOOL bResult = Super::Tick( DeltaTime, TickType );

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>( Player );
	if( LocalPlayer && LocalPlayer->PlayerPostProcess )
	{
		UPostProcessChain* Chain = LocalPlayer->PlayerPostProcess;
		for( INT NodeIndex = 0; NodeIndex < Chain->m_AllNodes.Num(); NodeIndex++ )
		{
			UArkPpNode* Node = Chain->m_AllNodes(NodeIndex);
			if( Node && Node->m_Controller )
			{
				Node->m_Controller->Tick( DeltaTime, TickType );
				GDisPpControllerTicks++;
			}
		}
	}

	if( DisPowerCensusEnabled() )
	{
		DisPowerReport( GWorld, DeltaTime );
	}
	if( DisLookAtCensusEnabled() )
	{
		DisLookAtReport( GWorld, DeltaTime );
	}
	if( DisTallboyCensusEnabled() )
	{
		DisTallboyReport( GWorld, DeltaTime );
	}
	return bResult;
}

/**
 * DISHONORED(port): 2013 rva 0x6adeb0 (2012 0x6eb900), 1,498 bytes.
 */
void ADishonoredPlayerController::ModifyPostProcessSettings( FArkPpConfig& Config )
{
	// DISHONORED(bringup): the switch of the measurement pair - the ACTOR feed contributes nothing, which is exactly
	// what this tree delivered before this function was ported.
	if( DisModifyPpSuppressed() )
	{
		return;
	}
	if( Pawn == NULL )
	{
		return;
	}
	GDisPpModifyCalls++;

	// DISHONORED(bringup): UpdateSunBlindingEffect (0x6a6450) is named in the block comment above and not ported.

	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( PpManager == NULL )
	{
		// retail's DisGetPpManager returns a reference and would fault here; there is no game info this early.
		return;
	}

	FPpBridge& Bridge = PpManager->m_PpBridge;
	if( !Bridge.m_bInitDone )
	{
		Bridge.m_UnderWater_BendTime_Blink_Pp = Cast<UArkPpNodeBlur>( DisGetArkPpNode( FName(TEXT("UnderWat_BendTime_Blink")) ) );
		Bridge.m_UnderWaterVectorsPp          = DisGetArkPpNodeMaterial( FName(TEXT("UnderWaterVectors")), FALSE );
		Bridge.m_BendTimeVectorsPp            = DisGetArkPpNodeMaterial( FName(TEXT("BendTimeVectors")), FALSE );
		// DISHONORED(retail): retail 2013 copies the BendTime node's uber parameters into the backup here, unguarded;
		// the 2012 build has no such copy. Guarded, because a missing node is exactly the case the tail tests for.
		if( Bridge.m_BendTimeVectorsPp )
		{
			Bridge.m_BendTimeVectorsUberBackup = Bridge.m_BendTimeVectorsPp->m_UberParameters;
		}
		Bridge.m_BendTimeUnderWaterVectorsPp  = DisGetArkPpNodeMaterial( FName(TEXT("BendTimeUnderWaterVectors")), FALSE );
		Bridge.m_BendTimeUnderWaterSwitch     = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("BendTimeUnderWaterSwitch")) ) );
		Bridge.m_BendTimeSwitch               = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("BendTimeSwitch")) ) );
		Bridge.m_BlinkVectorsPp               = DisGetArkPpNodeMaterial( FName(TEXT("BlinkVectors")), FALSE );
		Bridge.m_BlinkVectors2Pp              = DisGetArkPpNodeMaterial( FName(TEXT("BlinkVectors2")), FALSE );
		Bridge.m_MainBlinkSwitch              = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("BlinkSwitch")) ) );
		Bridge.m_BlinkOnlySwitch              = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("BlinkOnlySwitch")) ) );
		Bridge.m_AdrenalinePp                 = DisGetArkPpNodeMaterial( FName(TEXT("AdrenalineVectors")), FALSE );
		Bridge.m_AdrenalineSwitch             = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("AdrenalineSwitch")) ) );
		Bridge.m_PlagueVectors                = DisGetArkPpNodeMaterial( FName(TEXT("PlagueVectors")), FALSE );
		Bridge.m_PlagueSwitch                 = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("PlagueSwitch")) ) );
		Bridge.m_PossessionVectors            = DisGetArkPpNodeMaterial( FName(TEXT("PossessionVectors")), FALSE );
		Bridge.m_PossessionInMaterial         = DisGetArkPpNodeMaterial( FName(TEXT("PossessionIN")), FALSE );
		Bridge.m_PossessionOutMaterial        = DisGetArkPpNodeMaterial( FName(TEXT("PossessionOUT")), FALSE );
		Bridge.m_PossessionSwitch             = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("PossessionSwitch")) ) );
		Bridge.m_PpNodeAA                     = Cast<UArkPpNodeAA>( DisGetArkPpNode( FName(TEXT("Antialiasing")) ) );
		Bridge.m_KOBackupFrame                = DisGetArkPpNodeMaterial( FName(TEXT("BackupLastFrame")), FALSE );
		Bridge.m_KOBlendFrame                 = DisGetArkPpNodeMaterial( FName(TEXT("BlendLastFrame")), FALSE );
		Bridge.m_KOVectors                    = DisGetArkPpNodeMaterial( FName(TEXT("KOVectors")), FALSE );
		Bridge.m_KOSwitch                     = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("KOSwitch")) ) );
		Bridge.m_ZoomLensVectors              = DisGetArkPpNodeMaterial( FName(TEXT("LensVectors")), FALSE );
		Bridge.m_ZoomLensCompose              = DisGetArkPpNodeMaterial( FName(TEXT("LensCompose")), FALSE );
		Bridge.m_ZoomLensSwitch               = Cast<UArkPpNodeSwitch>( DisGetArkPpNode( FName(TEXT("LensSwitch")) ) );

		// retail's own test, in retail's own order: all twenty-four at once, and nothing is remembered until they
		// all resolve.
		if( Bridge.m_UnderWater_BendTime_Blink_Pp && Bridge.m_UnderWaterVectorsPp
			&& Bridge.m_BendTimeVectorsPp && Bridge.m_BendTimeUnderWaterVectorsPp
			&& Bridge.m_BendTimeUnderWaterSwitch && Bridge.m_BendTimeSwitch
			&& Bridge.m_BlinkVectorsPp && Bridge.m_BlinkVectors2Pp
			&& Bridge.m_MainBlinkSwitch && Bridge.m_BlinkOnlySwitch
			&& Bridge.m_AdrenalinePp && Bridge.m_AdrenalineSwitch
			&& Bridge.m_PlagueVectors && Bridge.m_PlagueSwitch
			&& Bridge.m_PossessionVectors && Bridge.m_PossessionInMaterial && Bridge.m_PossessionOutMaterial
			&& Bridge.m_PossessionSwitch && Bridge.m_PpNodeAA
			&& Bridge.m_KOBackupFrame && Bridge.m_KOBlendFrame && Bridge.m_KOVectors && Bridge.m_KOSwitch
			&& Bridge.m_ZoomLensVectors && Bridge.m_ZoomLensCompose && Bridge.m_ZoomLensSwitch )
		{
			Bridge.m_UnderWaterOriginalUberPpParameters = Bridge.m_UnderWaterVectorsPp->m_UberParameters;
			Bridge.m_bInitDone = TRUE;
			GDisPpBridgeInits++;
		}
		else
		{
			GDisPpBridgeNodesMissing++;
		}
	}

	const FLOAT DeltaSeconds = GWorld->IsPaused() ? 0.0f : GWorld->GetDeltaSeconds();

	ADishonoredPlayerCamera* DisCamera = Cast<ADishonoredPlayerCamera>( PlayerCamera );
	if( DisCamera )
	{
		// retail calls this through an unchecked cast of PlayerCamera
		DisCamera->ApplyCameraPostProcess( Config );
	}
	ApplyWaterPostProcessSettings( DeltaSeconds );

	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	if( PlayerPawn )
	{
		PlayerPawn->ApplyHealthEffectsPost( Config );
		// DISHONORED(bringup): ApplyAdrenalineProcessSettings (0x6a65e0) goes here, between the health effects and
		// dark vision.
		ApplyDarkVisionPostProcessSettings( PlayerPawn, DeltaSeconds );
		// DISHONORED(bringup): ApplyMusicalOverseerPostProcessSettings (0x6a6970) goes here.
	}
	// DISHONORED(bringup): UDisPostProcessManager::ApplyKismetPostProcessSettings (0x7ef9b0) goes here, before
	// the UI one.
	// DISHONORED(port, agent FA): 0x7efca0 - the interface's own channel, which is what blurs the scene behind a
	// message box. It runs on the real frame delta rather than the paused one, because a box that pauses the game
	// must still be able to fade its blur in.
	{
		UDisPostProcessManager* PpManager = DisGetPpManager();
		if( PpManager != NULL )
		{
			PpManager->ApplyUIPostProcessSettings( Config, GWorld->GetDeltaSeconds() );
		}
	}
	ApplyPossessionPostProcessSettings( DeltaSeconds );
}

/**
 * DISHONORED(port): 2013 rva 0x6a6750 (2012 0x6e2550) - the water volume's own grade. The underwater node's uber
 * parameters are restored from the backup the init block took and the volume's override is blended onto them at full
 * weight, then the Epp_Underwater effect is asked for (or dropped) so the manager's Tick shows or hides the node.
 *
 * DISHONORED(bringup): two calls of retail's body are left out and named.
 *   GWorld->Scene->SetBelowWaterTranslucentSort(bAbove) - FSceneInterface vtable slot 1 in retail; this tree's
 *     FSceneInterface has no such virtual and adding one at slot 1 would shift every slot after it. It only orders
 *     translucency against the water plane.
 *   the UDisFogComponent block (the volume info's m_FogComponent @392, its surface Z and the attach/detach bookkeeping
 *     through ADishonoredPlayerController::m_CurrentWaterFogComponent) - UDisFogComponent has no class declaration in
 *     this tree at all, only forward uses, so its two floats cannot be written.
 * Retail closes with DisGetGameInfo() and one call on it that the retail build has identical-code-folded with
 * UDishonoredTask_Base::OnAdded_Impl; the fold makes the real callee unidentifiable, so it is not guessed at.
 */
void ADishonoredPlayerController::ApplyWaterPostProcessSettings( FLOAT DeltaSeconds )
{
	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( PpManager == NULL )
	{
		return;
	}
	ADishonoredPawn* DisPawn = Cast<ADishonoredPawn>( Pawn );
	ADishonoredWaterVolume* WaterVolume = DisPawn ? DisPawn->m_pActiveWaterVolume : NULL;
	UDishonoredWaterVolumeInfo* VolumeInfo = WaterVolume ? WaterVolume->m_pWaterVolumeInfo : NULL;

	if( VolumeInfo )
	{
		// retail dereferences m_UnderWaterVectorsPp unchecked; only the copy is guarded, not the branch, because the
		// branch is what decides whether the player is in water at all
		UArkPpNodeMaterial* UnderWaterNode = PpManager->m_PpBridge.m_UnderWaterVectorsPp;
		if( UnderWaterNode )
		{
			UnderWaterNode->m_UberParameters = PpManager->m_PpBridge.m_UnderWaterOriginalUberPpParameters;
		}
		if( VolumeInfo->m_PpOverride.m_bOverrideUberPpParameters )
		{
			ArkUberPpSetDefaultOnNoOverride( VolumeInfo->m_PpOverride.m_UberPpParameters );
		}
		else
		{
			ArkUberPpForceDefault( VolumeInfo->m_PpOverride.m_UberPpParameters );
		}
		if( UnderWaterNode )
		{
			ArkUberPpApplyTo( VolumeInfo->m_PpOverride.m_UberPpParameters, UnderWaterNode->m_UberParameters, 1.0f, FALSE );
		}
		PpManager->StartEffect( Epp_Underwater, FALSE );
	}
	else if( PpManager->IsEffectRequired( Epp_Underwater ) )
	{
		PpManager->StopEffect( Epp_Underwater );
	}
}

/**
 * DISHONORED(port): 2013 rva 0x6a68d0 (2012 0x6d6d40) - dark vision. The whole function is one bit: the power
 * component says whether its post-process is active, and that bit goes onto UDisDarkVisionPpController::m_bIsActive,
 * which is what the controller's IsShown answers with and what its Tick fades on. The Epp_DarkVision request is
 * raised beside it; nothing in the manager's Tick reads that index, which is why dark vision is the one power whose
 * whole visible path is the controller.
 *
 * DISHONORED(bringup): -disdarkvisionpp holds the bit set, which is the single thing the power itself does. It is the
 * switch of the accept pair.
 */
void ADishonoredPlayerController::ApplyDarkVisionPostProcessSettings( ADishonoredPlayerPawn* PlayerPawn, FLOAT DeltaSeconds )
{
	const UBOOL bPpActive = DisDarkVisionForced()
		|| ( PlayerPawn && PlayerPawn->m_pDarkVisionPower && PlayerPawn->m_pDarkVisionPower->IsPpActive() );

	static FName DarkVisionNodeName( TEXT("DarkVisionPostProcess") );
	UArkPpNodeMaterial* Node = DisGetArkPpNodeMaterial( DarkVisionNodeName, FALSE );
	if( !Node || !Node->m_Controller )
	{
		return;
	}
	UDisDarkVisionPpController* Controller = Cast<UDisDarkVisionPpController>( Node->m_Controller );
	if( Controller )
	{
		Controller->m_bIsActive = bPpActive ? TRUE : FALSE;
	}
	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( PpManager == NULL )
	{
		return;
	}
	if( bPpActive )
	{
		if( !PpManager->IsEffectRequired( Epp_DarkVision ) )
		{
			PpManager->StartEffect( Epp_DarkVision, FALSE );
		}
	}
	else if( PpManager->IsEffectRequired( Epp_DarkVision ) )
	{
		PpManager->StopEffect( Epp_DarkVision );
	}
}

/**
 * DISHONORED(port): 2013 rva 0x6a6c30 (2012 0x6e2880) - possession, a four-stage machine on
 * m_PossessionEffectSettings.m_Stage driving UDisPostProcessManager::m_PossessionParams (R = distortion, G = blur,
 * B = the weight the possession node draws at). Stage 1 ramps both up over the intro, stage 2 waits for the possession
 * timer to fall under the warning threshold, stage 3 ramps to the warning values, stage 4 tears down.
 *
 * DISHONORED(retail): stage 1's ramps are `1 / m_IntroDuration * target * dt` - the rate is the target over the
 * duration, so a zero intro duration divides by zero exactly as retail does. Left as retail has it.
 */
void ADishonoredPlayerController::ApplyPossessionPostProcessSettings( FLOAT DeltaSeconds )
{
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( !PlayerPawn || !PlayerPawn->m_pPossessPower || PpManager == NULL )
	{
		return;
	}
	UDisTweaks_Possess* Tweaks = Cast<UDisTweaks_Possess>( PlayerPawn->m_pPossessPower->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_Possess::StaticClass()->GetDefaultObject<UDisTweaks_Possess>();
	}

	switch( m_PossessionEffectSettings.m_Stage )
	{
	case 0:
		PpManager->StopEffect( Epp_Possession );
		PpManager->StopEffect( Epp_PossessionIn );
		break;

	case 1:
		{
			if( !PpManager->IsEffectRequired( Epp_PossessionIn ) && !PpManager->IsEffectRequired( Epp_Possession ) )
			{
				PpManager->StartEffect( Epp_PossessionIn, FALSE );
			}
			const FLOAT Rate = 1.0f / Tweaks->m_IntroDuration;
			FLOAT Distort = PpManager->m_PossessionParams.R + Rate * Tweaks->m_WhileMaxDistort * DeltaSeconds;
			FLOAT Blur    = PpManager->m_PossessionParams.G + Rate * Tweaks->m_WhileMaxBlur * DeltaSeconds;
			Distort = Clamp( Distort, 0.0f, Tweaks->m_WhileMaxDistort );
			Blur    = Clamp( Blur, 0.0f, Tweaks->m_WhileMaxBlur );
			PpManager->m_PossessionParams.R = Distort;
			PpManager->m_PossessionParams.G = Blur;
			PpManager->m_PossessionParams.B = 1.0f;
			if( Distort >= Tweaks->m_WhileMaxDistort && Blur >= Tweaks->m_WhileMaxBlur )
			{
				m_PossessionEffectSettings.m_Stage = 2;
			}
		}
		break;

	case 2:
		// DISHONORED(port): UDishonoredActivePowerComponent_Possess::GetPossessionTimeLeft (2013 rva 0x7e7960,
		// 2012 0x849100) is a seven-byte getter for m_fPossessionTimer; read directly rather than adding a
		// CppText header for one accessor.
		if( Tweaks->m_WarnApparitionInSecondsBeforeForceExit > PlayerPawn->m_pPossessPower->m_fPossessionTimer )
		{
			m_PossessionEffectSettings.m_Stage = 3;
		}
		break;

	case 3:
		{
			const FLOAT Rate = 1.0f / Tweaks->m_WarnFadeDuration;
			FLOAT Distort = PpManager->m_PossessionParams.R + Rate * Tweaks->m_WarnMaxDistort * DeltaSeconds;
			FLOAT Blur    = PpManager->m_PossessionParams.G + Rate * Tweaks->m_WarnMaxBlur * DeltaSeconds;
			PpManager->m_PossessionParams.R = Clamp( Distort, 0.0f, Tweaks->m_WarnMaxDistort );
			PpManager->m_PossessionParams.G = Clamp( Blur, 0.0f, Tweaks->m_WarnMaxBlur );
			PpManager->m_PossessionParams.B = 1.0f;
		}
		break;

	case 4:
		m_PossessionEffectSettings.m_Stage = 0;
		if( PpManager->IsEffectRequired( Epp_PossessionIn ) )
		{
			PpManager->StopEffect( Epp_PossessionIn );
		}
		else
		{
			PpManager->StopEffect( Epp_Possession );
		}
		break;

	default:
		break;
	}
}

/*---------------------------------------------------------------------------
	-dispowerdbg: the census of the ACTOR feed. One line a second, so a run's log carries the whole history.
---------------------------------------------------------------------------*/

struct FDisPowerCensusState
{
	UWorld* World;
	FLOAT NextReportTime;
};
static FDisPowerCensusState GDisPowerState = { NULL, 0.0f };

void DisPowerReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World )
	{
		return;
	}
	if( GDisPowerState.World != World )
	{
		GDisPowerState.World = World;
		GDisPowerState.NextReportTime = 0.0f;
	}
	const FLOAT Now = World->GetTimeSeconds();
	if( Now < GDisPowerState.NextReportTime )
	{
		return;
	}
	GDisPowerState.NextReportTime = Now + 1.0f;

	UDisPostProcessManager* PpManager = DisGetPpManager();
	ULocalPlayer* LocalPlayer = GEngine && GEngine->GamePlayers.Num() > 0 ? GEngine->GamePlayers(0) : NULL;
	UPostProcessChain* Chain = LocalPlayer ? LocalPlayer->PlayerPostProcess : NULL;

	INT Nodes = 0;
	INT Controllers = 0;
	INT Shown = 0;
	if( Chain )
	{
		Nodes = Chain->m_AllNodes.Num();
		for( INT NodeIndex = 0; NodeIndex < Nodes; NodeIndex++ )
		{
			UArkPpNode* Node = Chain->m_AllNodes(NodeIndex);
			if( Node && Node->m_Controller )
			{
				Controllers++;
				if( Node->m_Controller->IsShown( Node ) )
				{
					Shown++;
				}
			}
		}
	}

	static FName DarkVisionNodeName( TEXT("DarkVisionPostProcess") );
	UArkPpNodeMaterial* DarkVisionNode = DisGetArkPpNodeMaterial( DarkVisionNodeName, FALSE );
	UDisDarkVisionPpController* DarkVision = DarkVisionNode ? Cast<UDisDarkVisionPpController>( DarkVisionNode->m_Controller ) : NULL;

	debugf( TEXT("DISHONORED(bringup): dispower census: manager %i bridgeInit %i (misses %i) modifyCalls %i | chain nodes %i, controllers %i, shown %i, controller ticks %i")
		, PpManager != NULL ? 1 : 0
		, ( PpManager && PpManager->m_PpBridge.m_bInitDone ) ? 1 : 0
		, GDisPpBridgeNodesMissing
		, GDisPpModifyCalls
		, Nodes, Controllers, Shown, GDisPpControllerTicks );

	if( DarkVisionNode )
	{
		debugf( TEXT("DISHONORED(bringup): dispower darkvision: node %s showInGame %i controller %s active %i debug %i eyeLid %.3f power %.3f shown %i material %s")
			, *DarkVisionNode->GetName()
			, (INT)DarkVisionNode->m_bShowInGame
			, DarkVision ? *DarkVision->GetClass()->GetName() : TEXT("NULL")
			, DarkVision ? (INT)DarkVision->m_bIsActive : -1
			, DarkVision ? (INT)DarkVision->m_bDebugIsActive : -1
			, DarkVision ? DarkVision->m_EyeLidTime : -1.0f
			, DarkVision ? DarkVision->m_PowerTime : -1.0f
			, ( DarkVision && DarkVision->IsShown( DarkVisionNode ) ) ? 1 : 0
			, DarkVisionNode->m_Material ? *DarkVisionNode->m_Material->GetName() : TEXT("NULL") );
	}
	else
	{
		debugf( TEXT("DISHONORED(bringup): dispower darkvision: no DarkVisionPostProcess node in the chain") );
	}

	if( PpManager )
	{
		FString Effects;
		for( INT Effect = 0; Effect < Epp_Count; Effect++ )
		{
			if( PpManager->m_RequiredEffects[Effect] || PpManager->m_EffectStates[Effect] )
			{
				Effects += FString::Printf( TEXT("%i:req%i/state%i "), Effect, PpManager->m_RequiredEffects[Effect], (INT)PpManager->m_EffectStates[Effect] );
			}
		}
		debugf( TEXT("DISHONORED(bringup): dispower effects: %s| tallboy stilts %i lights %i")
			, Effects.Len() ? *Effects : TEXT("none "), GDisTallboyStiltsSet, GDisTallboyLightsSpawned );
	}
}

// agentDO:lookatcensus
/*---------------------------------------------------------------------------
	-dislookat: the measurement behind agent DI's hand-over 2 ("the head is tilted back").

	A Dishonored character is two skeletal meshes that share one skeleton: the body on APawn::Mesh and the head on
	ADishonoredNPCPawn::m_pHeadMesh, bound to the body through ParentAnimComponent. The head can only be wrong two
	ways, and they need opposite fixes, so the census prints the SAME bone from BOTH meshes:

	  * head_jnt's component-space rotation on the body mesh - what the animation tree produced. FArkComponentLookat
	    (2013 rva 0x552640 OnComposeSkeleton and the 50 functions around it, all unported) would override exactly this
	    bone, so if the body's own head_jnt is already craned back, head-look is what hides it and the pose is what
	    causes it.
	  * head_jnt's component-space rotation on the head mesh - what the parent-anim composition produced. If the two
	    disagree, the composition is the defect and head-look has nothing to do with it.

	head_jnt / eye_L_jnt / eye_R_jnt / camera_jnt are retail's own names, read out of
	FArkComponentLookat::s_HeadBoneName and its siblings (2013 dynamic initialisers 0xb83720..0xb83800).
---------------------------------------------------------------------------*/

static INT GDisLookAtCensus = -1;

UBOOL DisLookAtCensusEnabled()
{
	if( GDisLookAtCensus < 0 )
	{
		GDisLookAtCensus = ( appStrfind( appCmdLine(), TEXT("-dislookat") ) != NULL ) ? 1 : 0;
	}
	return GDisLookAtCensus != 0;
}

static FString DisBoneRotationText( USkeletalMeshComponent* Component, INT BoneIndex )
{
	if( !Component || BoneIndex == INDEX_NONE )
	{
		return FString( TEXT("-") );
	}
	FString Text;
	if( Component->SpaceBases.IsValidIndex( BoneIndex ) )
	{
		const FRotator Space = Component->SpaceBases(BoneIndex).GetRotation().Rotator();
		const FVector Origin = Component->SpaceBases(BoneIndex).GetOrigin();
		Text += FString::Printf( TEXT("space(P%i Y%i R%i at %.0f,%.0f,%.0f)"), Space.Pitch, Space.Yaw, Space.Roll, Origin.X, Origin.Y, Origin.Z );
	}
	else
	{
		Text += TEXT("space(none)");
	}
	if( Component->LocalAtoms.IsValidIndex( BoneIndex ) )
	{
		const FRotator Local = Component->LocalAtoms(BoneIndex).GetRotation().Rotator();
		Text += FString::Printf( TEXT(" local(P%i Y%i R%i)"), Local.Pitch, Local.Yaw, Local.Roll );
	}
	else
	{
		Text += TEXT(" local(none)");
	}
	return Text;
}

static void DisLookAtReportMesh( const TCHAR* Label, USkeletalMeshComponent* Component, FName BoneName )
{
	if( !Component || !Component->SkeletalMesh )
	{
		debugf( TEXT("DISHONORED(bringup): dislookat %s: no component/mesh"), Label );
		return;
	}
	const INT BoneIndex = Component->SkeletalMesh->MatchRefBone( BoneName );
	debugf( TEXT("DISHONORED(bringup): dislookat %s %s: bones %i, spaceBases %i, localAtoms %i, required %i, refpose %i, parentAnim %s, parentBoneMap %i | %s %i %s"),
		Label,
		*Component->SkeletalMesh->GetName(),
		Component->SkeletalMesh->RefSkeleton.Num(),
		Component->SpaceBases.Num(),
		Component->LocalAtoms.Num(),
		Component->RequiredBones.Num(),
		(INT)Component->bForceRefpose,
		Component->ParentAnimComponent ? *Component->ParentAnimComponent->GetName() : TEXT("NULL"),
		Component->ParentBoneMap.Num(),
		*BoneName.ToString(),
		BoneIndex,
		*DisBoneRotationText( Component, BoneIndex ) );
}

struct FDisLookAtCensusState
{
	UWorld* World;
	FLOAT NextReportTime;
};
static FDisLookAtCensusState GDisLookAtState = { NULL, 0.0f };

void DisLookAtReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World )
	{
		return;
	}
	if( GDisLookAtState.World != World )
	{
		GDisLookAtState.World = World;
		GDisLookAtState.NextReportTime = 0.0f;
	}
	const FLOAT Now = World->GetTimeSeconds();
	if( Now < GDisLookAtState.NextReportTime )
	{
		return;
	}
	GDisLookAtState.NextReportTime = Now + 2.0f;

	static const FName HeadBoneName( TEXT("head_jnt") );
	static const FName NeckBoneName( TEXT("neck_jnt") );

	INT Pawns = 0;
	ADishonoredNPCPawn* First = NULL;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredNPCPawn* NPCPawn = Cast<ADishonoredNPCPawn>( *It );
		if( !NPCPawn || NPCPawn->IsPendingKill() )
		{
			continue;
		}
		Pawns++;
		if( !First && NPCPawn->Mesh && NPCPawn->m_pHeadMesh )
		{
			First = NPCPawn;
		}
	}
	if( !First )
	{
		debugf( TEXT("DISHONORED(bringup): dislookat: %i NPC pawns, none with both meshes"), Pawns );
		return;
	}
	debugf( TEXT("DISHONORED(bringup): dislookat census: %i NPC pawns, first %s, anim tree %s, lookat requests 0 (FArkComponentLookat unported)"),
		Pawns, *First->GetName(),
		First->Mesh && First->Mesh->AnimTreeTemplate ? *First->Mesh->AnimTreeTemplate->GetName() : TEXT("NULL") );
	DisLookAtReportMesh( TEXT("body"), First->Mesh, HeadBoneName );
	DisLookAtReportMesh( TEXT("head"), First->m_pHeadMesh, HeadBoneName );
	DisLookAtReportMesh( TEXT("body-neck"), First->Mesh, NeckBoneName );
	DisLookAtReportMesh( TEXT("head-neck"), First->m_pHeadMesh, NeckBoneName );

	// agentDO:bindposedelta - how far each bone's ANIMATED local rotation is from its own bind pose, on the body
	// mesh, which is the mesh the renderer reads (UpdateRefToLocalMatrices takes the head's matrices from the parent
	// through ParentBoneMap whenever the map is complete, so the head component's own SpaceBases are never drawn).
	// A pose in which one bone is tens of degrees off its bind pose and its neighbours are not is not a pose: it is
	// that bone's animation channel.
	if( First->Mesh && First->Mesh->SkeletalMesh )
	{
		USkeletalMeshComponent* Body = First->Mesh;
		FString Line;
		for( INT BoneIndex = 0; BoneIndex < Body->SkeletalMesh->RefSkeleton.Num() && BoneIndex < 24; BoneIndex++ )
		{
			if( !Body->LocalAtoms.IsValidIndex(BoneIndex) )
			{
				continue;
			}
			const FQuat Animated = Body->LocalAtoms(BoneIndex).GetRotation();
			const FQuat Bind = Body->SkeletalMesh->RefSkeleton(BoneIndex).BonePos.Orientation;
			FLOAT Dot = Animated | Bind;
			if( Dot < 0.f )
			{
				Dot = -Dot;
			}
			Dot = Clamp( Dot, -1.f, 1.f );
			const FLOAT Degrees = 2.f * appAcos( Dot ) * 180.f / PI;
			Line += FString::Printf( TEXT("%s %.0f; "), *Body->SkeletalMesh->RefSkeleton(BoneIndex).Name.ToString(), Degrees );
		}
		debugf( TEXT("DISHONORED(bringup): dislookat body bind-pose delta (degrees): %s"), *Line );

		// agentDO:skelcontrols - the only other thing that can rotate a bone after the tree has blended it.
		UAnimTree* Tree = Cast<UAnimTree>( Body->Animations );
		if( Tree )
		{
			FString Controls;
			for( INT ListIndex = 0; ListIndex < Tree->SkelControlLists.Num(); ListIndex++ )
			{
				const FSkelControlListHead& Head = Tree->SkelControlLists(ListIndex);
				Controls += FString::Printf( TEXT("%s:"), *Head.BoneName.ToString() );
				INT Depth = 0;
				for( USkelControlBase* Control = Head.ControlHead; Control && Depth < 8; Control = Control->NextControl, Depth++ )
				{
					Controls += FString::Printf( TEXT("%s(%.2f)"), *Control->GetClass()->GetName(), Control->ControlStrength );
				}
				Controls += TEXT("; ");
			}
			debugf( TEXT("DISHONORED(bringup): dislookat body skel controls (%i lists): %s"), Tree->SkelControlLists.Num(), *Controls );

			// agentDO:animnodes - an aim offset is the other way a tree rotates the neck and the head, and it takes
			// its Aim from the pawn every frame; a tree whose Aim nobody feeds sits at whatever the node was cooked
			// with. Printed with the sequences that actually carry weight, so the pose has a source.
			TArray<UAnimNode*> Nodes;
			Tree->GetNodes( Nodes );
			FString Aims;
			FString Playing;
			for( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
			{
				UAnimNodeAimOffset* Aim = Cast<UAnimNodeAimOffset>( Nodes(NodeIndex) );
				if( Aim )
				{
					Aims += FString::Printf( TEXT("%s aim(%.3f,%.3f) offset(%.3f,%.3f) forced %i dir %i weight %.2f profile %i/%i; "),
						*Aim->NodeName.ToString(), Aim->Aim.X, Aim->Aim.Y, Aim->AngleOffset.X, Aim->AngleOffset.Y,
						(INT)Aim->bForceAimDir, (INT)Aim->ForcedAimDir, Aim->NodeTotalWeight,
						Aim->CurrentProfileIndex, Aim->Profiles.Num() );
				}
				UAnimNodeSequence* Seq = Cast<UAnimNodeSequence>( Nodes(NodeIndex) );
				if( Seq && Seq->NodeTotalWeight > 0.01f )
				{
					Playing += FString::Printf( TEXT("%s=%s(%.2f@%.2f); "), *Seq->NodeName.ToString(),
						*Seq->AnimSeqName.ToString(), Seq->NodeTotalWeight, Seq->CurrentTime );
				}
			}
			debugf( TEXT("DISHONORED(bringup): dislookat body %i anim nodes; aim offsets: %s"), Nodes.Num(), Aims.Len() ? *Aims : TEXT("none") );
			debugf( TEXT("DISHONORED(bringup): dislookat body sequences with weight: %s"), Playing.Len() ? *Playing : TEXT("none") );

			// agentDO:nodetable - every node with its class, so the thing that holds the head-aim additives has a name
			// and a class, and the children of every blend so its weights are visible.
			FString Table;
			for( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
			{
				UAnimNode* Node = Nodes(NodeIndex);
				Table += FString::Printf( TEXT("%s:%s(%.2f) "), *Node->NodeName.ToString(), *Node->GetClass()->GetName(), Node->NodeTotalWeight );
			}
			debugf( TEXT("DISHONORED(bringup): dislookat body node table: %s"), *Table );

			// agentDO:animstable - a UAnimNodeSequenceBlendBase blends its Anims array by per-entry weight
			// (UnAnimPlay.cpp:2201). UArkAnimNodeLookAt is the head-aim blender and it is the only thing that ever
			// writes those weights, out of its aim cursor; unported, the array keeps whatever the cook left.
			for( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
			{
				UAnimNodeSequenceBlendBase* BlendBase = Cast<UAnimNodeSequenceBlendBase>( Nodes(NodeIndex) );
				if( !BlendBase || BlendBase->Anims.Num() == 0 )
				{
					continue;
				}
				FString Entries;
				for( INT AnimIndex = 0; AnimIndex < BlendBase->Anims.Num(); AnimIndex++ )
				{
					const FAnimBlendInfo& Info = BlendBase->Anims(AnimIndex);
					Entries += FString::Printf( TEXT("%s=%.2f "), *Info.AnimInfo.AnimSeqName.ToString(), Info.Weight );
				}
				debugf( TEXT("DISHONORED(bringup): dislookat blendbase %s:%s weight %.2f seq %s anims(%i): %s"),
					*BlendBase->NodeName.ToString(), *BlendBase->GetClass()->GetName(), BlendBase->NodeTotalWeight,
					*BlendBase->AnimSeqName.ToString(), BlendBase->Anims.Num(), *Entries );
			}
			for( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
			{
				UAnimNodeBlendBase* Blend = Cast<UAnimNodeBlendBase>( Nodes(NodeIndex) );
				if( !Blend || Blend->Children.Num() == 0 )
				{
					continue;
				}
				FString Kids;
				for( INT ChildIndex = 0; ChildIndex < Blend->Children.Num(); ChildIndex++ )
				{
					const FAnimBlendChild& Child = Blend->Children(ChildIndex);
					Kids += FString::Printf( TEXT("%s=%.2f/%.2f%s "), *Child.Name.ToString(), Child.Weight, Child.BlendWeight,
						Child.bIsAdditive ? TEXT("(add)") : TEXT("") );
				}
				debugf( TEXT("DISHONORED(bringup): dislookat blend %s:%s weight %.2f children: %s"),
					*Blend->NodeName.ToString(), *Blend->GetClass()->GetName(), Blend->NodeTotalWeight, *Kids );
			}
		}
		else
		{
			debugf( TEXT("DISHONORED(bringup): dislookat body skel controls: Animations is %s, not a UAnimTree"),
				Body->Animations ? *Body->Animations->GetClass()->GetName() : TEXT("NULL") );
		}
	}
}

// agentDO:tallboyprobe
/*---------------------------------------------------------------------------
	-distallboy: the tallboy is the only ADishonoredNPCPawn subclass with an appearance pass of its own, and the
	content that carries one is cooked seek-free into whichever level uses it. The census says, for the map actually
	loaded: how many tallboy pawns exist, how many tallboy tweak objects and pawn archetypes are loaded at all, and
	whether m_pStiltsMesh has a USkeletalMesh on it - the same "mesh set" measurement agent DI used for the head.

	-distallboyspawn=<seconds> spawns one from the first loaded archetype in front of the player, so a map that holds
	the content but places no tallboy still exercises both passes.
---------------------------------------------------------------------------*/

static INT GDisTallboyCensus = -1;

UBOOL DisTallboyCensusEnabled()
{
	if( GDisTallboyCensus < 0 )
	{
		GDisTallboyCensus = ( appStrfind( appCmdLine(), TEXT("-distallboy") ) != NULL ) ? 1 : 0;
	}
	return GDisTallboyCensus != 0;
}

struct FDisTallboyCensusState
{
	UWorld* World;
	FLOAT NextReportTime;
	UBOOL bSpawnTried;
};
static FDisTallboyCensusState GDisTallboyState = { NULL, 0.0f, FALSE };

void DisTallboyReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World )
	{
		return;
	}
	if( GDisTallboyState.World != World )
	{
		GDisTallboyState.World = World;
		GDisTallboyState.NextReportTime = 0.0f;
		GDisTallboyState.bSpawnTried = FALSE;
	}
	const FLOAT Now = World->GetTimeSeconds();

	FLOAT SpawnTime = 0.0f;
	const UBOOL bWantSpawn = Parse( appCmdLine(), TEXT("distallboyspawn="), SpawnTime ) && SpawnTime > 0.0f;
	if( bWantSpawn && !GDisTallboyState.bSpawnTried && Now >= SpawnTime )
	{
		GDisTallboyState.bSpawnTried = TRUE;
		ADisTallboyNPCPawn* Archetype = NULL;
		for( TObjectIterator<ADisTallboyNPCPawn> It; It; ++It )
		{
			if( It->IsTemplate() )
			{
				Archetype = *It;
				break;
			}
		}
		ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
		if( Archetype && PlayerPawn )
		{
			const FVector Forward = PlayerPawn->Rotation.Vector();
			const FVector Where = PlayerPawn->Location + Forward * 400.f + FVector(0, 0, 100.f);
			AActor* Spawned = World->SpawnActor( Archetype->GetClass(), NAME_None, Where, PlayerPawn->Rotation, Archetype, TRUE );
			debugf( TEXT("DISHONORED(bringup): distallboy spawn from archetype %s at %.0f,%.0f,%.0f -> %s"),
				*Archetype->GetPathName(), Where.X, Where.Y, Where.Z, Spawned ? *Spawned->GetName() : TEXT("FAILED") );
		}
		else
		{
			debugf( TEXT("DISHONORED(bringup): distallboy spawn: no loaded ADisTallboyNPCPawn archetype (player %i)"), PlayerPawn != NULL ? 1 : 0 );
		}
	}

	// agentDO:tallboycam - -distallboycam=<seconds> puts the first tallboy that has its stilts in front of the
	// player's own view point, because the two on L_Boyle_Ext_P stand far from the spawn anchor and a run that
	// starts there never has one in shot. Same shape as agent DI's -disheadcam, and a tallboy is three metres tall,
	// so the framing distance and drop are its own.
	FLOAT CamTime = 0.0f;
	if( Parse( appCmdLine(), TEXT("distallboycam="), CamTime ) && CamTime > 0.0f && Now >= CamTime
		&& ADishonoredPlayerController::s_pInstance )
	{
		ADisTallboyNPCPawn* Target = NULL;
		for( TObjectIterator<ADisTallboyNPCPawn> It; It; ++It )
		{
			if( It->IsTemplate() || It->IsPendingKill() )
			{
				continue;
			}
			if( !Target || ( It->m_pStiltsMesh && It->m_pStiltsMesh->SkeletalMesh ) )
			{
				Target = *It;
			}
			if( Target && Target->m_pStiltsMesh && Target->m_pStiltsMesh->SkeletalMesh )
			{
				break;
			}
		}
		if( Target )
		{
			FVector CamLocation( 0.f, 0.f, 0.f );
			FRotator CamRotation( 0, 0, 0 );
			ADishonoredPlayerController::s_pInstance->GetPlayerViewPoint( CamLocation, CamRotation );
			const FVector Forward = CamRotation.Vector();
			const FVector Destination = CamLocation + Forward * 520.f - FVector( 0.f, 0.f, 150.f );
			Target->Physics = PHYS_None;
			Target->Velocity = FVector( 0.f, 0.f, 0.f );
			World->FarMoveActor( Target, Destination, FALSE, TRUE );
			FRotator Facing = ( CamLocation - Destination ).Rotation();
			Facing.Pitch = 0;
			Facing.Roll = 0;
			Target->Rotation = Facing;
		}
	}

	if( Now < GDisTallboyState.NextReportTime )
	{
		return;
	}
	GDisTallboyState.NextReportTime = Now + 2.0f;

	INT Tweaks = 0;
	FString TweakNames;
	for( TObjectIterator<UDisTweaks_TallboyNPCPawn> It; It; ++It )
	{
		Tweaks++;
		if( Tweaks <= 4 )
		{
			TweakNames += FString::Printf( TEXT("%s(stilts %s) "), *It->GetName(),
				It->m_pStiltsSkeletalMesh ? *It->m_pStiltsSkeletalMesh->GetName() : TEXT("NULL") );
		}
	}
	INT Archetypes = 0;
	INT Instances = 0;
	INT WithStiltsMesh = 0;
	INT WithLight = 0;
	ADisTallboyNPCPawn* First = NULL;
	for( TObjectIterator<ADisTallboyNPCPawn> It; It; ++It )
	{
		if( It->IsTemplate() )
		{
			Archetypes++;
			continue;
		}
		if( It->IsPendingKill() )
		{
			continue;
		}
		Instances++;
		if( !First )
		{
			First = *It;
		}
		if( It->m_pStiltsMesh && It->m_pStiltsMesh->SkeletalMesh )
		{
			WithStiltsMesh++;
		}
		if( It->m_pAttachedLight )
		{
			WithLight++;
		}
	}
	debugf( TEXT("DISHONORED(bringup): distallboy census: %i tallboy pawns (%i archetypes), stilts mesh set %i, attached light %i; tweak objects %i %s| passes: ApplyTweakChanges %i, light spawns %i"),
		Instances, Archetypes, WithStiltsMesh, WithLight, Tweaks, Tweaks ? *TweakNames : TEXT(""),
		GDisTallboyStiltsSet, GDisTallboyLightsSpawned );
	if( First )
	{
		debugf( TEXT("DISHONORED(bringup): distallboy first %s: body %s, stilts %s attached %i parentAnim %s, head %s, light %s"),
			*First->GetName(),
			First->Mesh && First->Mesh->SkeletalMesh ? *First->Mesh->SkeletalMesh->GetName() : TEXT("NULL"),
			First->m_pStiltsMesh && First->m_pStiltsMesh->SkeletalMesh ? *First->m_pStiltsMesh->SkeletalMesh->GetName() : TEXT("NULL"),
			First->m_pStiltsMesh ? (INT)First->m_pStiltsMesh->IsAttached() : -1,
			First->m_pStiltsMesh && First->m_pStiltsMesh->ParentAnimComponent ? *First->m_pStiltsMesh->ParentAnimComponent->GetName() : TEXT("NULL"),
			First->m_pHeadMesh && First->m_pHeadMesh->SkeletalMesh ? *First->m_pHeadMesh->SkeletalMesh->GetName() : TEXT("NULL"),
			First->m_pAttachedLight ? *First->m_pAttachedLight->GetName() : TEXT("NULL") );
	}
}

// agentDO:lookataim
/*---------------------------------------------------------------------------
	The head-aim blend node's rest state. See Inc/CppText/UArkAnimNodeLookAt.h for what is measured, what retail
	does and why this is a stand-in rather than a port. The bodies live here, beside the -dislookat census that
	found the defect, because UArkAnimNodeLookAt is declared in DishonoredGameEngineShims.h and Engine cannot see
	it; their real home is Engine/Src/arkanimnodelookat.cpp once the node is ported into Engine.
---------------------------------------------------------------------------*/

static INT GDisLookAtAimOff = -1;
static INT GDisLookAtPlayer = -1;

UBOOL DisLookAtAimStandInEnabled()
{
	if( GDisLookAtAimOff < 0 )
	{
		GDisLookAtAimOff = ( appStrfind( appCmdLine(), TEXT("-nodislookataim") ) != NULL ) ? 1 : 0;
	}
	return GDisLookAtAimOff == 0;
}

UBOOL DisLookAtPlayerEnabled()
{
	if( GDisLookAtPlayer < 0 )
	{
		GDisLookAtPlayer = ( appStrfind( appCmdLine(), TEXT("-dislookatplayer") ) != NULL ) ? 1 : 0;
	}
	return GDisLookAtPlayer != 0;
}

/**
 * The eighteen weights from a normalised aim. The grid is column-major in the cooked array: for each of the two
 * sets (head 0..8, torso 9..17) the columns are left, centre, right and the rows within a column are up, centre,
 * down. A bilinear weight over the four cells the aim falls between reproduces the shape of retail's own mapping
 * without its ranges; at (0,0) it is exactly the Center pose, which is the rest state the node is authored around.
 */
void UArkAnimNodeLookAt::DisSetNormalisedAim( const FVector2D& NormalisedAim )
{
	if( Anims.Num() < 9 )
	{
		return;
	}
	const FLOAT Col = Clamp( NormalisedAim.X, -1.f, 1.f ) + 1.f;   // 0 left .. 2 right
	const FLOAT Row = 1.f - Clamp( NormalisedAim.Y, -1.f, 1.f );   // 0 up   .. 2 down
	const INT Col0 = Clamp( appTrunc(Col), 0, 1 );
	const INT Row0 = Clamp( appTrunc(Row), 0, 1 );
	const FLOAT ColFrac = Col - (FLOAT)Col0;
	const FLOAT RowFrac = Row - (FLOAT)Row0;

	for( INT AnimIndex = 0; AnimIndex < Anims.Num(); AnimIndex++ )
	{
		Anims(AnimIndex).Weight = 0.f;
	}
	const FLOAT CellWeights[4] = { (1.f - ColFrac) * (1.f - RowFrac), (1.f - ColFrac) * RowFrac,
	                               ColFrac * (1.f - RowFrac),         ColFrac * RowFrac };
	const INT CellIndices[4] = { (Col0 + 0) * 3 + (Row0 + 0), (Col0 + 0) * 3 + (Row0 + 1),
	                             (Col0 + 1) * 3 + (Row0 + 0), (Col0 + 1) * 3 + (Row0 + 1) };
	for( INT Cell = 0; Cell < 4; Cell++ )
	{
		if( Anims.IsValidIndex( CellIndices[Cell] ) )
		{
			Anims( CellIndices[Cell] ).Weight += CellWeights[Cell];
		}
	}
}

void UArkAnimNodeLookAt::TickAnim( FLOAT DeltaSeconds )
{
	if( DisLookAtAimStandInEnabled() )
	{
		FVector2D Aim( 0.f, 0.f );
		if( DisLookAtPlayerEnabled() && SkelComponent && SkelComponent->GetOwner() && ADishonoredPlayerPawn::s_pInstance )
		{
			AActor* Owner = SkelComponent->GetOwner();
			const FVector ToTarget = ADishonoredPlayerPawn::s_pInstance->Location - ( Owner->Location + FVector(0, 0, 60.f) );
			const FRotator ToTargetRotation = ToTarget.Rotation();
			const FRotator Relative = ( ToTargetRotation - Owner->Rotation ).GetNormalized();
			// one grid step is a quarter turn of the head, which is what the authored corner poses are
			Aim.X = Clamp( (FLOAT)Relative.Yaw / 16384.f, -1.f, 1.f );
			Aim.Y = Clamp( (FLOAT)Relative.Pitch / 16384.f, -1.f, 1.f );
		}
		m_Aim = Aim;
		DisSetNormalisedAim( Aim );
	}
	UAnimNodeSequence::TickAnim( DeltaSeconds );
}

// agentDO:togglesprint
/*---------------------------------------------------------------------------
	The sprint key, ported after the merge gate caught its native firing unbound (unported_natives 0 -> 1).

	DISHONORED(port): 2013 rva 0x6af250 (2012 0x6f8690), 219 bytes. The tweaks decide whether the sprint key is a
	toggle or a hold, and they decide it PER SOURCE: m_bUseSprintToggle for the keyboard and mouse,
	m_bUseSprintToggle_GamePad for the pad. In toggle mode the key starts a sprint when the pawn is not sprinting and
	stops one when it is - but only from the keyboard, because retail's pad branch has no stop. In hold mode the two
	m_bSprintEnabled_* flags are simply set and the movement code reads them.

	DISHONORED(retail): m_bHeldSprintCanceledSneak is cleared on every path out of this function, including the ones
	that do nothing else, and m_bSprintEnabled_FromToggle is only ever cleared by the hold branch.
---------------------------------------------------------------------------*/

// agentDO:sprintdbg - -dissprintdbg says whether the sprint key reaches this at all, with what, and from where.
// Read on first use, never in a file-scope static (agent CA's finding).
static INT GDisSprintDbg = -1;
INT GDisToggleSprintCalls = 0;

static UBOOL DisSprintDbgEnabled()
{
	if( GDisSprintDbg < 0 )
	{
		GDisSprintDbg = ( appStrfind( appCmdLine(), TEXT("-dissprintdbg") ) != NULL ) ? 1 : 0;
	}
	return GDisSprintDbg != 0;
}

void ADishonoredPlayerController::DisToggleSprint( UBOOL bFromGamePad )
{
	GDisToggleSprintCalls++;
	UDishonoredPlayerInput* DisInput = Cast<UDishonoredPlayerInput>( PlayerInput );
	UDisTweaks_PlayerInput* Tweaks = DisInput ? Cast<UDisTweaks_PlayerInput>( DisInput->GetTweaks_Derived() ) : NULL;
	if( !Tweaks )
	{
		// retail dereferences PlayerInput unchecked and falls back to the class default when the tweak object is absent
		Tweaks = UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject<UDisTweaks_PlayerInput>();
	}

	const UBOOL bToggleMode = ( !bFromGamePad && Tweaks->m_bUseSprintToggle )
		|| ( Tweaks->m_bUseSprintToggle_GamePad && bFromGamePad );
	ADishonoredPawn* DisPawn = Cast<ADishonoredPawn>( Pawn );

	if( bToggleMode )
	{
		if( DisPawn && !DisPawn->m_bSprinting )
		{
			m_bSprintEnabled_FromGamePad = bFromGamePad ? TRUE : FALSE;
			m_bSprintEnabled_FromToggle = TRUE;
			DisPawn->SetSprinting( TRUE );
		}
		else if( !bFromGamePad && DisPawn )
		{
			// retail has no matching stop for the pad: a pad toggle only ever starts a sprint
			DisPawn->SetSprinting( FALSE );
		}
	}
	else
	{
		m_bSprintEnabled_FromGamePad = bFromGamePad ? TRUE : FALSE;
		m_bSprintEnabled_FromToggle = FALSE;
	}
	m_bHeldSprintCanceledSneak = FALSE;

	if( DisSprintDbgEnabled() && GDisToggleSprintCalls <= 8 )
	{
		debugf( TEXT("DISHONORED(bringup): dissprint census: call %i, fromGamePad %i, toggleMode %i, pawn %s, sprinting %i, fromGamePadFlag %i, fromToggleFlag %i"),
			GDisToggleSprintCalls, (INT)bFromGamePad, (INT)bToggleMode,
			DisPawn ? *DisPawn->GetName() : TEXT("NULL"),
			DisPawn ? (INT)DisPawn->m_bSprinting : -1,
			(INT)m_bSprintEnabled_FromGamePad, (INT)m_bSprintEnabled_FromToggle );
	}
}

/** DISHONORED(port): 2013 rva 0x5ee690 - one UBOOL parameter, no return. */
void ADishonoredPlayerController::execDisToggleSprint( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(bFromGamePad);
	P_FINISH;
	DisToggleSprint( bFromGamePad );
}

// DISHONORED(port): agent EQ. Retail's body is empty, in both builds: the 2012 interface vtable
// (ADishonoredPlayerController{for IArkSettingsListenerInterface} @0xd16598, slot 1) and the 2013 one
// (0xd18730 +4) both point at a three-byte `ret 8` that /OPT:ICF shares with every other empty
// two-argument virtual - the PDB happens to name that fold UGameViewportClient::SetOnlyUseControllerTiltInput.
// The controller's own copies of the settings are written by UDishonoredPlayerInput::ApplyGameSettings.
void ADishonoredPlayerController::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
}
