// DishonoredGame/src/disseqcond_checkstoryflag.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb290  public: static void __cdecl UDisSeqCond_CheckStoryFlag::InitializePrivateStaticClassUDisSeqCond_CheckStoryFlag(void)
//   0x7d1930  public: virtual void __thiscall UDisSeqCond_CheckStoryFlag::Activated(void)
//   0x7e9de0  public: static class UClass * __cdecl UDisSeqCond_CheckStoryFlag::GetPrivateStaticClassUDisSeqCond_CheckStoryFlag(wchar_t const *)
//   0x7edb70  public: static class UClass * __cdecl UDisSeqCond_CheckStoryFlag::StaticClassNoInline(void)

// ---- agent EL (PHASE12 EL) ----
#include "DishonoredGame.h"

// DISHONORED(port): agent EL, 2013 rva 0x78e9d0 (2012 0x7d1930). The flag has to be declared by the set AND
// recorded on the player pawn: a GUID the set does not know is 'False' whatever the pawn holds.
// ADishonoredPlayerPawn::s_pInstance is dereferenced without a test in retail and is dereferenced without one
// here, because the only path that reaches the second test has already found the flag in a loaded set.
void UDisSeqCond_CheckStoryFlag::Activated()
{
	UBOOL bValue = FALSE;
	if( m_pStoryFlagSet && m_pStoryFlagSet->FindStoryFlag_ByGUID( m_StoryFlag ) )
	{
		bValue = ADishonoredPlayerPawn::s_pInstance->CheckStoryFlag( m_pStoryFlagSet, m_StoryFlag );
	}
	OutputLinks( bValue ? 0 : 1 ).bHasImpulse = TRUE;
}
