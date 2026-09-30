// DishonoredGame/src/dissquadinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file, 2012 rvas:
//   0x644490  public: static unsigned int __cdecl IDisSquadInterface::IsSquadSupported(class TArray<struct FDisSquadProperty, class FDefaultAllocator> const &, class FName const &)

// ---- agent EP ports (PHASE14 EP): the squad filter every route, spawner and flee volume shares ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x5fe910 (2012 0x644490): an EMPTY filter accepts everybody; a non-empty one accepts only
// a name it lists. There is no NAME_None wildcard - retail compares the two halves of the FName and nothing else - so an
// entry left blank in the editor excludes every squad but the blank one.
UBOOL IDisSquadInterface::IsSquadSupported( const TArray<FDisSquadProperty>& _rSupportedSquads, const FName& _rSquadName )
{
	if( _rSupportedSquads.Num() == 0 )
	{
		return TRUE;
	}
	for( INT Idx = 0; Idx < _rSupportedSquads.Num(); Idx++ )
	{
		if( _rSupportedSquads(Idx).m_SquadName == _rSquadName )
		{
			return TRUE;
		}
	}
	return FALSE;
}
