// DishonoredGame/src/disconv_playerlookatspeaker.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x8fef10  public: virtual void __thiscall UDisConv_PlayerLookAtSpeaker::GetActionText(class FString &)const
//   0x907940  protected: virtual int __thiscall UDisConv_PlayerLookAtSpeaker::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x90cd20  public: static class UClass * __cdecl UDisConv_PlayerLookAtSpeaker::GetPrivateStaticClassUDisConv_PlayerLookAtSpeaker(wchar_t const *)
//   0x90df10  public: static void __cdecl UDisConv_PlayerLookAtSpeaker::InitializePrivateStaticClassUDisConv_PlayerLookAtSpeaker(void)
//   0x90e970  public: static class UClass * __cdecl UDisConv_PlayerLookAtSpeaker::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent JE (PHASE24 JE): the conversation node that turns the player's view onto one of the conversation's speakers - the
	same rules as UDisConv_PlayerLookAtActor: a conversation that blocks the player, the dialog state current (until then
	waiting), a pawn tracked by hit region, any other actor at its bounds.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkJE_UDisConv_PlayerLookAtSpeaker()
{
}

// 2013 0x8acc80 (305 B; 2012 0x907940): 2013 also waits while the player is falling
INT UDisConv_PlayerLookAtSpeaker::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UDisConversation_InGameData* InGameConv = _pRunningInstance->GetInGameConv( this );
	if( !m_pOwner || !m_pOwner->m_bIsBlocking_Player || m_iSpeakerToLookAt == -1 || !InGameConv )
	{
		return 0;
	}
	IDisConvSpeakerInterface* Other = InGameConv->GetSpeaker( m_iSpeakerToLookAt );
	if( !Other )
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
	AActor* OtherActor = Cast<AActor>( Other->GetUObjectInterfaceDisConvSpeakerInterface() );
	if( !OtherActor )
	{
		return 0;
	}
	const FDisConvLookAtConstraints* Constraints = m_bUseLookConstraints ? &m_LookConstraints : NULL;
	if( ADishonoredPawn* OtherPawn = Cast<ADishonoredPawn>( OtherActor ) )
	{
		InDialog->TrackPawnForDialog( OtherPawn, m_fBlendTime, m_HitRegionToLookAt, Constraints );
		return 0;
	}
	InDialog->TrackActorForDialog( OtherActor, m_fBlendTime, Constraints );
	return 0;
}

// 2013 0x8a7980 (53 B; 2012 0x8f8a50): the play again, its waiting flag dropped
INT UDisConv_PlayerLookAtSpeaker::TickNode( FLOAT _fDeltaTime, FDisDialogRunningInstance* _pRunningInstance ) const
{
	UBOOL bNotFinished = FALSE;
	return PlayNode_Derived( bNotFinished, _pRunningInstance );
}
