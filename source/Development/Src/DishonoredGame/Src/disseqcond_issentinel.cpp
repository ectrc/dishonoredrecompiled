// DishonoredGame/src/disseqcond_issentinel.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7cb3b0  public: static void __cdecl UDisSeqCond_IsSentinel::InitializePrivateStaticClassUDisSeqCond_IsSentinel(void)
//   0x7d1fd0  public: virtual void __thiscall UDisSeqCond_IsSentinel::Activated(void)
//   0x7ea2f0  public: static class UClass * __cdecl UDisSeqCond_IsSentinel::GetPrivateStaticClassUDisSeqCond_IsSentinel(wchar_t const *)
//   0x7edd20  public: static class UClass * __cdecl UDisSeqCond_IsSentinel::StaticClassNoInline(void)

// ---- agent EL (PHASE12 EL) ----
#include "DishonoredGame.h"

// DISHONORED(port): agent EL, 2013 rva 0x78f070 (2012 0x7d1fd0). "Sentinel" is the engine's automated
// performance-capture harness, and AGameInfo::MyAutoTestManager is what drives it: retail's test is
// literally `GWorld->GetGameInfo()->MyAutoTestManager != NULL`, so in a played session this condition
// always takes 'False'. USequenceCondition does not auto-activate its output links, which is why the
// whole chain downstream of it stops dead when this body is missing.
void UDisSeqCond_IsSentinel::Activated()
{
	AGameInfo* pGameInfo = GWorld->GetGameInfo();
	if( pGameInfo && pGameInfo->MyAutoTestManager )
	{
		OutputLinks(0).bHasImpulse = TRUE;
	}
	else
	{
		OutputLinks(1).bHasImpulse = TRUE;
	}
}
