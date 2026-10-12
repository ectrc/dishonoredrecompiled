// DishonoredGame/src/disconv_conversationref.cpp
// DISHONORED(port): agent FR (PHASE17 FR). PlayNode_Derived 2012 rva 0x8f6610 (304 B). InitNode 0x8f2080 and
// OnDelete_Derived 0x8f2190 are editor-only (agentFR.md inventory). The 2012 body tests the referenced conversation's
// 0x200 / 0x400 flag bits, which in the 2013 layout this tree declares are m_bFireConvOnlyOnce and m_bNotWhenPlayerIsInCombat
// (2013 inserted m_bAlwaysInterruptForCombat and m_bCanStartDuringEnemyAwareness before them).
#include "DishonoredGame.h"

INT UDisConv_ConversationRef::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	if( m_OutputLinks.Num() <= 0 )
	{
		return 1;
	}
	UDisConversation* Conv = Cast<UDisConversation>( m_OutputLinks(0).m_pLink );
	if( !Conv )
	{
		return 1;
	}
	if( Conv->m_bFireConvOnlyOnce && ADishonoredPlayerPawn::s_pInstance )
	{
		FDisConvSaveData* Global = ADishonoredPlayerPawn::s_pInstance->GetDialogSaveData_Global();
		if( Global && Global->FindConvData( Conv->GetDialogTree(), Conv, FALSE ) )
		{
			return 1;
		}
	}
	// DISHONORED(bringup): retail also refuses a m_bNotWhenPlayerIsInCombat conversation while ADishonoredPlayerPawn::IsInCombat();
	// the player's combat state is the combat package's.
	UDisConversation_InGameData* ConvData = Cast<UDisConversation_InGameData>( _pRunningInstance->GetInGameNode( Conv, eDisConvInGameDataType_PerNode ) );
	if( ConvData && Conv->m_OutputLinks.Num() > 0 && Conv->m_OutputLinks(0).m_pLink )
	{
		if( _pRunningInstance->IsCheckOnly() )
		{
			_pRunningInstance->m_CheckOnlyResult = ConvData->AcquireSpeakersCheckOnly( _pRunningInstance ) ? eDisDialogCheckOnly_Found : eDisDialogCheckOnly_Found_But_Busy;
		}
		else if( ConvData->AcquireSpeakers( _pRunningInstance ) )
		{
			return 0;
		}
	}
	return 1;
}
