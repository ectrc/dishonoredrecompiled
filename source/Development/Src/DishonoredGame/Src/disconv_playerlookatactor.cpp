// DishonoredGame/src/disconv_playerlookatactor.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x8f8a50  public: virtual int __thiscall UDisConv_PlayerLookAtSpeaker::TickNode(float, struct FDisDialogRunningInstance *)const
//   0x8fed80  public: virtual void __thiscall UDisConv_PlayerLookAtActor::GetActionText(class FString &)const
//   0x9077b0  protected: virtual int __thiscall UDisConv_PlayerLookAtActor::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x90cc90  public: static class UClass * __cdecl UDisConv_PlayerLookAtActor::GetPrivateStaticClassUDisConv_PlayerLookAtActor(wchar_t const *)
//   0x90def0  public: static void __cdecl UDisConv_PlayerLookAtActor::InitializePrivateStaticClassUDisConv_PlayerLookAtActor(void)
//   0x90e940  public: static class UClass * __cdecl UDisConv_PlayerLookAtActor::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent JE (PHASE24 JE): the conversation node that turns the player's view onto an actor. It only acts in a conversation
	that blocks the player, and only once the player's dialog state is the current master state: until then it waits (not
	finished, output -1) and its TickNode tries again each tick. A spawner names its first spawned pawn, tracked by hit region;
	any other actor is tracked at its bounds. The node's own constraints go with it when it uses them.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "jediag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkJE_UDisConv_PlayerLookAtActor()
{
}

// 2013 0x8acac0 (385 B; 2012 0x9077b0): 2013 also waits while the player is falling
INT UDisConv_PlayerLookAtActor::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	if( !m_pLookAtActor )
	{
		return 0;
	}
	if( !m_pOwner || !m_pOwner->m_bIsBlocking_Player )
	{
		return 0;
	}
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( Player->m_pPlayerMasterFSM->GetLogicalState() );
	if( !InDialog )
	{
		return 0;
	}
	if( InDialog != Player->m_pPlayerMasterFSM->GetCurrentlyActiveState() || Player->Physics == PHYS_Falling )
	{
		_rbNotFinished = TRUE;
		return -1;
	}
	const FDisConvLookAtConstraints* Constraints = m_bUseLookConstraints ? &m_LookConstraints : NULL;
	ADishonoredSpawner* Spawner = Cast<ADishonoredSpawner>( m_pLookAtActor );
	if( Spawner && Spawner->m_SpawnedPawns.Num() && Spawner->m_SpawnedPawns(0) )
	{
		InDialog->TrackPawnForDialog( Spawner->m_SpawnedPawns(0), m_fBlendTime, m_HitRegionToLookAt, Constraints );
		return 0;
	}
	InDialog->TrackActorForDialog( m_pLookAtActor, m_fBlendTime, Constraints );
	return 0;
}

// 2013 0x8a7980 (53 B; 2012 0x8f8a50): the play again, its waiting flag dropped
INT UDisConv_PlayerLookAtActor::TickNode( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UBOOL bNotFinished = FALSE;
	return PlayNode_Derived( bNotFinished, _pRunningInstance );
}
