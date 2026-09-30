// DishonoredGame/src/dishonorednavpoint.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4), 2012 rvas:
//   0x6917d0  public: static void __cdecl ADishonoredNavPoint::InitializePrivateStaticClassADishonoredNavPoint(void)
//   0x697b10  public: unsigned int __thiscall ADishonoredNavPoint::IsGuardPoint(void)const
//   0x6a80a0  public: static class UClass * __cdecl ADishonoredNavPoint::GetPrivateStaticClassADishonoredNavPoint(wchar_t const *)
//   0x6a88f0  public: static class UClass * __cdecl ADishonoredNavPoint::StaticClassNoInline(void)

// ---- agent EP ports (PHASE14 EP): the one predicate that tells a patrol point from a guard post ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x64bab0 (2012 0x697b10): a point is a guard post if it says to stand there forever, or
// for a while, or if it names any watch point. The watch-point arm is the one that is not obvious: an EMPTY entry in
// m_WatchPoints makes the whole test FALSE rather than skipping that entry, because a guard post with a dangling watch
// point would send the guard to look at nothing. The offsets read here (m_bGuardForever at 804, m_fGuardDuration at
// 784, m_WatchPoints at 792) are the generated header's reflected span 784..808.
UBOOL ADishonoredNavPoint::IsGuardPoint() const
{
	if( m_bGuardForever || m_fGuardDuration > 0.f )
	{
		return TRUE;
	}
	if( m_WatchPoints.Num() <= 0 )
	{
		return FALSE;
	}
	for( INT Idx = 0; Idx < m_WatchPoints.Num(); Idx++ )
	{
		if( !m_WatchPoints(Idx) )
		{
			return FALSE;
		}
	}
	return TRUE;
}
