// DishonoredGame/src/dismusicstate_exploration.cpp
// PDB functions attributed to this file (4):
//   0x8b17f0  public: virtual void __thiscall UDisMusicState_Exploration::OnEnterState(class UDishonoredNativeState *)
//   0x8b8e20  public: static class UClass * __cdecl UDisMusicState_Exploration::GetPrivateStaticClassUDisMusicState_Exploration(wchar_t const *)
//   0x8bc8c0  public: static void __cdecl UDisMusicState_Exploration::InitializePrivateStaticClassUDisMusicState_Exploration(void)
//   0x8c0f90  public: static class UClass * __cdecl UDisMusicState_Exploration::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile (the 2013 body is unnamed in match_2012_2013.csv).

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateExploration()
{
}

/** 2012 0x8b17f0: the map's m_pStartExploration event on the player pawn. */
void UDisMusicState_Exploration::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	DisMusicPostStateEvent( TEXT("Exploration"), m_pMusicEvents->m_pStartExploration );
}
