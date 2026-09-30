// DishonoredGame/src/disseqact_setstoryflag.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb6d0  public: static void __cdecl UDisSeqAct_SetStoryFlag::InitializePrivateStaticClassUDisSeqAct_SetStoryFlag(void)
//   0x7d2bf0  public: virtual void __thiscall UDisSeqAct_SetStoryFlag::Activated(void)
//   0x7eb050  public: static class UClass * __cdecl UDisSeqAct_SetStoryFlag::GetPrivateStaticClassUDisSeqAct_SetStoryFlag(wchar_t const *)
//   0x7ee2a0  public: static class UClass * __cdecl UDisSeqAct_SetStoryFlag::StaticClassNoInline(void)

// ---- agent EL (PHASE12 EL) ----
#include "DishonoredGame.h"

// DISHONORED(port): agent EL, 2013 rva 0x78fce0 (2012 0x7d2bf0). Three input links - Set, Clear, Toggle - and
// the first one carrying an impulse decides; an activation with no impulsed input (retail's sentinel value 3)
// writes nothing. USequenceAction::Activated runs on every path, including the ones that write nothing.
void UDisSeqAct_SetStoryFlag::Activated()
{
	if( m_pStoryFlagSet && m_pStoryFlagSet->FindStoryFlag_ByGUID( m_StoryFlag ) )
	{
		ADishonoredPlayerPawn* pPlayerPawn = ADishonoredPlayerPawn::s_pInstance;
		if( pPlayerPawn )
		{
			INT Which = 3;
			for( INT LinkIndex = 0; LinkIndex < InputLinks.Num(); LinkIndex++ )
			{
				if( InputLinks(LinkIndex).bHasImpulse )
				{
					Which = LinkIndex;
					break;
				}
			}
			if( Which == 0 )
			{
				pPlayerPawn->SetStoryFlag( m_pStoryFlagSet, m_StoryFlag, TRUE );
			}
			else if( Which == 1 )
			{
				pPlayerPawn->SetStoryFlag( m_pStoryFlagSet, m_StoryFlag, FALSE );
			}
			else if( Which == 2 )
			{
				const UBOOL bCurrent = pPlayerPawn->CheckStoryFlag( m_pStoryFlagSet, m_StoryFlag );
				pPlayerPawn->SetStoryFlag( m_pStoryFlagSet, m_StoryFlag, !bCurrent );
			}
		}
	}
	USequenceAction::Activated();
}
