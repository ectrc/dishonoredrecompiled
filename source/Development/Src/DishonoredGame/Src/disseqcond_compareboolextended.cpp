// DishonoredGame/src/disseqcond_compareboolextended.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb2b0  public: static void __cdecl UDisSeqCond_CompareBoolExtended::InitializePrivateStaticClassUDisSeqCond_CompareBoolExtended(void)
//   0x7d6cb0  public: virtual void __thiscall UDisSeqCond_CompareBoolExtended::Activated(void)
//   0x7e9e70  public: static class UClass * __cdecl UDisSeqCond_CompareBoolExtended::GetPrivateStaticClassUDisSeqCond_CompareBoolExtended(wchar_t const *)
//   0x7edba0  public: static class UClass * __cdecl UDisSeqCond_CompareBoolExtended::StaticClassNoInline(void)

// ---- agent FK (PHASE15 FK) ----
// The addresses above are the 2012 ones the importer wrote. The body below was read out of the 2012
// Shipping binary, where it is PDB-named at 0x7d6cb0 and 540 bytes. Its 2013 counterpart is
// rva 0x79b3e0, 521 bytes, and the 2013 database leaves it as sub_B9B3E0: match_2012_2013.csv maps it
// by "neighbours" at ratio 0.838 and the sizes differ, so that address is NOT independently pinned and
// nothing here rests on it.
//
// -fkdiag counts DisSeqCond_CompareBoolExtended x3 in l_tower_p's nine levels with 0 activations on the
// opening route, so this one is not on the first mission's path; it is ported because it is the only
// other condition of the ten whose body needs nothing that is itself unported.
#include "DishonoredGame.h"

/**
 * DISHONORED(port): agent FK, 2012 rva 0x7d6cb0. All of the connected Bool variables, folded two ways
 * at once: m_bResultAnd starts TRUE and m_bResultOr FALSE, and the three output links are
 * 0 "all of them", 2 "some of them", 1 "none of them" - in that order, which is not the order of the
 * links. Both results are left on the object, which is why they are reflected properties.
 */
void UDisSeqCond_CompareBoolExtended::Activated()
{
	m_bResultAnd = TRUE;
	m_bResultOr = FALSE;

	TArray<UBOOL*> BoolVars;
	GetBoolVars( BoolVars, TEXT("Bool") );
	for( INT Index = 0; Index < BoolVars.Num(); Index++ )
	{
		const UBOOL bValue = *BoolVars( Index );
		m_bResultAnd = ( m_bResultAnd && bValue ) ? TRUE : FALSE;
		m_bResultOr = ( m_bResultOr || bValue ) ? TRUE : FALSE;
	}

	if( m_bResultAnd )
	{
		OutputLinks( 0 ).ActivateOutputLink();
	}
	else if( m_bResultOr )
	{
		OutputLinks( 2 ).ActivateOutputLink();
	}
	else
	{
		OutputLinks( 1 ).ActivateOutputLink();
	}
}

/** DISHONORED(bringup): agent FK. The unit's external-linkage anchor; see disseqcond_comparetweaks.cpp. */
void DishonoredLinkDisSeqCondCompareBoolExtended()
{
}
