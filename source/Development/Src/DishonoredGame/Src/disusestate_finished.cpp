// DishonoredGame/src/disusestate_finished.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x72ac40  public: static class UClass * __cdecl UDisUseState_Finished::GetPrivateStaticClassUDisUseState_Finished(wchar_t const *)
//   0x72db30  public: static void __cdecl UDisUseState_Finished::InitializePrivateStaticClassUDisUseState_Finished(void)
//   0x72fef0  public: static class UClass * __cdecl UDisUseState_Finished::StaticClassNoInline(void)
//   0x731bc0  public: __thiscall FDisUseState_Finished_Param::FDisUseState_Finished_Param(void)
//   0x734fe0  public: virtual void __thiscall UDisUseState_Finished::TickState(float)

// DISHONORED(port): agent GD (PHASE19 GD). TickState 2013 rva 0x6d5970 (2012 0x734fe0, 140 B): the state waits for the Use button
// to be released, then goes back to WaitForInput and tells the crosshair actor's interactable (interface slot +48, unnamed
// in both databases - left out) that the button went up.
#include "DishonoredGame.h"
#include "disusestate.h"

void UDisUseState_Finished::TickState( FLOAT _fDeltaSeconds )
{
	ADishonoredPlayerController* PC = m_pPlayerController;
	if( !PC || PC->m_bUseButton )
	{
		return;
	}
	FDisUseState_WaitForInput_Param Param;
	DemandStateChange( Param );
}
