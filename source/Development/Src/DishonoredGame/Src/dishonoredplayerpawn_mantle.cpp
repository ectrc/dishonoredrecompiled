// DishonoredGame/src/dishonoredplayerpawn_mantle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x6fb830  public: unsigned int __thiscall ADishonoredPlayerPawn::IsMantleAllowed(void)const
//   0x706f00  private: void __thiscall ADishonoredPlayerPawn::DoMantle_StepUp(float, class FVector const &, int)
//   0x707170  private: unsigned int __thiscall ADishonoredPlayerPawn::IsMantleRequested(void)const
//   0x7071a0  private: unsigned int __thiscall ADishonoredPlayerPawn::IsPressingForward(void)const
//   0x70d380  private: void __thiscall ADishonoredPlayerPawn::DoMantle_Anim(float, class FVector const &, int, unsigned int, unsigned int, unsigned int)
//   0x70ff10  public: unsigned int __thiscall ADishonoredPlayerPawn::PerformMantle(unsigned int, unsigned int)
//   0x7129d0  private: void __thiscall ADishonoredPlayerPawn::Tick_Mantle(float, enum ELevelTick)
//   0x712b10  private: unsigned int __thiscall ADishonoredPlayerPawn::DoJump_Mantle(void)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): the jump's half of the mantle. Agent IG owns the mantle itself (PerformMantle, Tick_Mantle,
	DoMantle_StepUp, DoMantle_Anim, IsMantleAllowed, the edge finder): until then PerformMantle refuses every request, so a
	jump is always a jump.
	2012 / 2013 rvas: DoJump_Mantle 0x712b10 / 0x6dbe70 (83 B both), IsMantleRequested 0x707170 / 0x6c8000, IsPressingForward
	0x7071a0 / 0x6c8030, PerformMantle 0x70ff10 / 0x6d78d0.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_PlayerPawnMantle()
{
}

// 2013 0x6dbe70: with an edge finder the mantle is allowed and, while the jump button is held, attempted; without one it is not
UBOOL ADishonoredPlayerPawn::DoJump_Mantle()
{
	if( m_pMantleEdgeFinder == NULL )
	{
		m_bMantleAllowed = FALSE;
		return FALSE;
	}
	m_bMantleAllowed = TRUE;
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->m_bJumpButton && PerformMantle( TRUE, FALSE );
}

// 2013 0x6c8000 (37 B)
UBOOL ADishonoredPlayerPawn::IsMantleRequested() const
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->m_bJumpButton;
}

// 2013 0x6c8030 (55 B)
UBOOL ADishonoredPlayerPawn::IsPressingForward() const
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC != NULL && PC->PlayerInput != NULL && PC->PlayerInput->RawJoyUp > 0.f;
}

// DISHONORED(bringup): 2012 0x70ff10 (283 B) is agent IG's package; until it lands no mantle is performed
UBOOL ADishonoredPlayerPawn::PerformMantle( UBOOL bFromJump, UBOOL bFromBlink )
{
	static UBOOL bSaid = FALSE;
	if( !bSaid )
	{
		bSaid = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): if ADishonoredPlayerPawn::PerformMantle (2012 0x70ff10) is agent IG's and not ported: no mantle") );
	}
	return FALSE;
}
