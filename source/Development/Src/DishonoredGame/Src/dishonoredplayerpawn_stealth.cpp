// DishonoredGame/src/dishonoredplayerpawn_stealth.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x6fb890  class ADisStealthVolume * & __cdecl DishonoredPlayerPawn_Stealth::GetNextStealthVolume(class ADisStealthVolume * const)
//   0x6fb8a0  public: virtual float __thiscall ADishonoredPlayerPawn::GetSpeedFraction(void)const
//   0x6fec60  public: virtual void __thiscall ADishonoredPlayerPawn::RefreshSearchProxy(void)
//   0x6fec70  public: unsigned int __thiscall ADishonoredPlayerPawn::IsSliding(void)const
//   0x6feca0  protected: unsigned int __thiscall ADishonoredPlayerPawn::AttemptChangeCollisionHeight(float)
//   0x707310  public: virtual void __thiscall ADishonoredPlayerPawn::OnEnterStealthVolume(class ADisStealthVolume *)
//   0x707380  public: virtual void __thiscall ADishonoredPlayerPawn::OnExitStealthVolume(class ADisStealthVolume *)
//   0x707400  public: void __thiscall ADishonoredPlayerPawn::ClearVisAndMantleSettings(void)
//   0x707470  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsSneaking(void)const
//   0x7074c0  protected: unsigned int __thiscall ADishonoredPlayerPawn::AttemptChangeToSneakHeight(void)
//   0x707510  protected: void __thiscall ADishonoredPlayerPawn::GameSave_Stealth(class FArchive &, enum ESaveLoadLocation)
//   0x70d440  public: virtual void __thiscall ADishonoredPlayerPawn::ExtractSearchLocation(class ADishonoredNPCPawn * const, class FVector &)
//   0x70d450  public: void __thiscall ADishonoredPlayerPawn::StartSneaking(unsigned int)
//   0x70d570  public: void __thiscall ADishonoredPlayerPawn::StopSneaking(unsigned int)
//   0x70d6a0  protected: void __thiscall ADishonoredPlayerPawn::GameLoad_Stealth(class FArchive &, enum ESaveLoadLocation)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): StopSneaking, 2012 0x70d570 (304 B) / 2013 0x6cd960 (neighbours 0.590). Retail: only while
	m_bSneakModeToggled (+2208 bit 5); AttemptChangeCollisionHeight( the class default's standing collision height ); on
	success the sneak bit and bWantsToCrouch (+640 bit 3 in retail's numbering: bIsCrouched) are cleared and, unless
	holding a movable, carrying a corpse or asked to skip transitions, the stand-up upper-body action is requested through
	the upper FSM (FStatePlayerAction_Param on m_pActionTweaks' stand actions) and the anim state invalidated.
	DISHONORED(bringup): AttemptChangeCollisionHeight (0x6feca0), the upper FSM and the action tweaks are not ported; what
	is ported is the state the rest of the soiree path reads: the sneak-mode bit and bIsCrouched cleared. Nothing sneaks
	on the unattended route, so this is reached with the bit clear and does nothing.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PlayerPawnStealth()
{
}

void ADishonoredPlayerPawn::StopSneaking( UBOOL bSkipTransitions )
{
	if( !m_bSneakModeToggled )
	{
		return;
	}
	static UBOOL bSaid = FALSE;
	if( !bSaid )
	{
		bSaid = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredPlayerPawn::StopSneaking: AttemptChangeCollisionHeight and the stand-up action through the upper FSM are not ported; only the sneak bits are cleared") );
	}
	m_bSneakModeToggled = FALSE;
	bIsCrouched = FALSE;
}
