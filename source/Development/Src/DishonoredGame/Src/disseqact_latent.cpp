// DishonoredGame/src/disseqact_latent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4), 2012 rvas:
//   0x7cb560  public: static void __cdecl UDisSeqAct_Latent::InitializePrivateStaticClassUDisSeqAct_Latent(void)
//   0x7d8be0  protected: void __thiscall UDisSeqAct_Latent::CleanUpLatentActors(void)
//   0x7ea9b0  public: static class UClass * __cdecl UDisSeqAct_Latent::GetPrivateStaticClassUDisSeqAct_Latent(wchar_t const *)
//   0x7edf10  public: static class UClass * __cdecl UDisSeqAct_Latent::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(port): 2012 rva 0x7d8be0 (unnamed in the 2013 database; UDisSeqAct_AIGoToActor::DeActivated 2013 0x7a2070 jumps
// to it): every actor still latent on this action forgets it, and the list is emptied.
void UDisSeqAct_Latent::CleanUpLatentActors()
{
	for( INT Idx = 0; Idx < LatentActors.Num(); Idx++ )
	{
		AActor* Actor = LatentActors(Idx);
		if( Actor )
		{
			Actor->LatentActions.RemoveItem( this );
		}
	}
	LatentActors.Empty();
}
