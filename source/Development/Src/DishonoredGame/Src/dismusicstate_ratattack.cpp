// DishonoredGame/src/dismusicstate_ratattack.cpp
// PDB functions attributed to this file (4):
//   0x8b1820  public: virtual void __thiscall UDisMusicState_RatAttack::OnEnterState(class UDishonoredNativeState *)
//   0x8b8eb0  public: static class UClass * __cdecl UDisMusicState_RatAttack::GetPrivateStaticClassUDisMusicState_RatAttack(wchar_t const *)
//   0x8bc8e0  public: static void __cdecl UDisMusicState_RatAttack::InitializePrivateStaticClassUDisMusicState_RatAttack(void)
//   0x8c0fc0  public: static class UClass * __cdecl UDisMusicState_RatAttack::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile (the 2013 body is unnamed in match_2012_2013.csv).

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateRatAttack()
{
}

/** 2012 0x8b1820: the map's m_pStartRatAttack event on the player pawn. */
void UDisMusicState_RatAttack::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	DisMusicPostStateEvent( TEXT("RatAttack"), m_pMusicEvents->m_pStartRatAttack );
}
