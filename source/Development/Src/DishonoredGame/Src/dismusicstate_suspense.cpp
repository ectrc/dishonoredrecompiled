// DishonoredGame/src/dismusicstate_suspense.cpp
// PDB functions attributed to this file (4):
//   0x8b1850  public: virtual void __thiscall UDisMusicState_Suspense::OnEnterState(class UDishonoredNativeState *)
//   0x8b8f40  public: static class UClass * __cdecl UDisMusicState_Suspense::GetPrivateStaticClassUDisMusicState_Suspense(wchar_t const *)
//   0x8bc900  public: static void __cdecl UDisMusicState_Suspense::InitializePrivateStaticClassUDisMusicState_Suspense(void)
//   0x8c0ff0  public: static class UClass * __cdecl UDisMusicState_Suspense::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile (the 2013 body is unnamed in match_2012_2013.csv).

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateSuspense()
{
}

/** 2012 0x8b1850: the map's m_pStartSuspense event on the player pawn. */
void UDisMusicState_Suspense::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	DisMusicPostStateEvent( TEXT("Suspense"), m_pMusicEvents->m_pStartSuspense );
}
