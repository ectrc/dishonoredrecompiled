// DishonoredGame/src/disusestate_holster.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x72acd0  public: static class UClass * __cdecl UDisUseState_Holster::GetPrivateStaticClassUDisUseState_Holster(wchar_t const *)
//   0x72ad60  public: virtual void __thiscall UDisUseState_Holster::OnEnterState(class UDishonoredNativeState *)
//   0x72db50  public: static void __cdecl UDisUseState_Holster::InitializePrivateStaticClassUDisUseState_Holster(void)
//   0x72ff20  public: static class UClass * __cdecl UDisUseState_Holster::StaticClassNoInline(void)
//   0x731bf0  public: __thiscall FDisUseState_Holster_Param::FDisUseState_Holster_Param(void)
//   0x735070  public: virtual void __thiscall UDisUseState_Holster::TickState(float)

// DISHONORED(port): agent GD (PHASE19 GD). OnEnterState 2013 rva 0x6cca00 (2012 0x72ad60, 102 B): the activation time is now plus
// the player-input tweaks' hold time. TickState 0x6d5a00 (0x735070, 416 B): while Use is held and the weapons can be holstered,
// an alt-interactable under the crosshair takes over, otherwise once the activation time has passed the weapons are
// holstered and the state finishes; Use released before that reloads an unloaded ranged weapon and finishes; weapons that
// cannot be holstered send the state back to WaitForInput.
// DISHONORED(bringup): CanHolsterWeapons / HolsterWeapons and the reload (UDishonoredWeapon_Ranged, AttemptUse_Reload) are
// unported: the state goes back to WaitForInput while Use is held and finishes when it is released.
#include "DishonoredGame.h"
#include "disusestate.h"

void UDisUseState_Holster::OnEnterState( UDishonoredNativeState* _pLastState )
{
	UDishonoredPlayerInput* Input = m_pPlayerController ? Cast<UDishonoredPlayerInput>( m_pPlayerController->PlayerInput ) : NULL;
	const UDisTweaks_PlayerInput* Tweaks = Input ? Input->m_pTweaks : NULL;
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_PlayerInput*)UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject();
	}
	m_fTimeOfHolsterActivation = ( GWorld ? GWorld->GetTimeSeconds() : 0.f ) + ( Tweaks ? Tweaks->m_fUnequipButtonTime : 0.f );
}

void UDisUseState_Holster::TickState( FLOAT _fDeltaSeconds )
{
	ADishonoredPlayerController* PC = m_pPlayerController;
	if( !PC )
	{
		return;
	}
	if( PC->m_bUseButton )
	{
		FDisUseState_WaitForInput_Param Param;
		DemandStateChange( Param );
		return;
	}
	FDisUseState_Finished_Param Finished;
	DemandStateChange( Finished );
}
