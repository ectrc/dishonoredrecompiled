/*=============================================================================
	AkLEngineCmds.cpp - CAkLEngineCmds (aklenginecmds.cpp, 2012 0x93e3b0..0x93ed60, 0x91c770; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26). No deviation.
=============================================================================*/
#include "AkLEngineCmds.h"
#include "AkLEngine.h"
#include "AkVPLSrcCbxNode.h"

CAkLEngineCmds::AkListCmd			CAkLEngineCmds::m_listCmd;
AkListBare<CAkVPLSrcCbxNode>		CAkLEngineCmds::m_listSrcsNotConnected;
AkUInt32							CAkLEngineCmds::m_ulPlayEventID = 0;
bool								CAkLEngineCmds::m_bProcessPlayCmdsNeeded = false;

/** 2012 0x93e3b0: 512 commands pre-allocated in the lower engine pool. */
AKRESULT CAkLEngineCmds::Init()
{
	m_ulPlayEventID = 0;
	return m_listCmd.Init( 512, (AkUInt32)-1 );
}

/** 2012 0x93ed40. */
void CAkLEngineCmds::Term()
{
	m_listSrcsNotConnected.RemoveAll();
	m_listCmd.Term();
}

/** 2012 0x93e5a0: a play (or play-paused) also asks the audio manager to connect it within this frame. */
AKRESULT CAkLEngineCmds::EnqueueAction( LEState in_eState, CAkPBI * in_pContext )
{
	AkLECmd * pCmd = m_listCmd.AddLast();
	if( !pCmd )
	{
		return AK_Fail;
	}
	pCmd->m_eState = in_eState;
	pCmd->m_pCtx = in_pContext;
	pCmd->m_ulSequenceNumber = m_ulPlayEventID;
	pCmd->m_bSourceConnected = false;
	if( in_eState <= LEStatePlayPause )
	{
		m_bProcessPlayCmdsNeeded = true;
	}
	return AK_Success;
}

/** 2012 0x93e5f0: a stop before the voice ever connected cancels the play instead (the PBI is destroyed as a failed play). */
AKRESULT CAkLEngineCmds::EnqueueActionStop( CAkPBI * in_pCtx )
{
	for( AkListCmd::ListItem * pItem = m_listCmd.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.m_pCtx != in_pCtx )
		{
			continue;
		}
		if( pItem->Item.m_eState <= LEStatePlayPause && !in_pCtx->m_pCbx )
		{
			DequeuePBI( in_pCtx );
			in_pCtx->Destroy( CtxDestroyReasonPlayFailed );
			return AK_Success;
		}
		break;
	}
	AkLECmd * pCmd = m_listCmd.AddLast();
	if( !pCmd )
	{
		return AK_Fail;
	}
	pCmd->m_pCtx = in_pCtx;
	pCmd->m_eState = LEStateStop;
	pCmd->m_ulSequenceNumber = m_ulPlayEventID;
	pCmd->m_bSourceConnected = false;
	return AK_Success;
}

/** 2012 0x93e500. */
void CAkLEngineCmds::DequeuePBI( CAkPBI * in_pPBI )
{
	AkListCmd::ListItem * pPrev = NULL;
	AkListCmd::ListItem * pItem = m_listCmd.m_pFirst;
	while( pItem )
	{
		AkListCmd::ListItem * pNext = pItem->pNextListItem;
		if( pItem->Item.m_pCtx == in_pPBI )
		{
			m_listCmd.RemoveItem( pItem, pPrev );
		}
		else
		{
			pPrev = pItem;
		}
		pItem = pNext;
	}
}

/** 2012 0x93ec10. */
void CAkLEngineCmds::DeleteAllCommandsForSource( CAkVPLSrcCbxNode * in_pCbx )
{
	DequeuePBI( in_pCbx->GetContext() );
}

/** 2012 0x93e670: new plays get their voice now; a failed AddSound drops the command. */
void CAkLEngineCmds::ProcessPlayCommands()
{
	AkListCmd::ListItem * pPrev = NULL;
	AkListCmd::ListItem * pItem = m_listCmd.m_pFirst;
	while( pItem )
	{
		if( pItem->Item.m_pCtx->m_pCbx || pItem->Item.m_eState > LEStatePlayPause || CAkLEngine::AddSound( pItem->Item ) == AK_Success )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		AkListCmd::ListItem * pNext = pItem->pNextListItem;
		m_listCmd.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
	m_bProcessPlayCmdsNeeded = false;
}

/** 2012 0x93e740: commands of one sequence number apply together, once every source of the sequence is connected
    (a source still waiting for its stream format holds back its whole sequence). */
void CAkLEngineCmds::ProcessPendingCommands()
{
StartOverProcCmds:
	AkListCmd::ListItem * pPrev = NULL;
	AkListCmd::ListItem * pItem = m_listCmd.m_pFirst;
	while( pItem )
	{
		const AkUInt32 ulCurSeqNumber = pItem->Item.m_ulSequenceNumber;
		bool bDoSkipSeqNumber = false;
		AkListCmd::ListItem * pSeqPrev = pPrev;
		for( AkListCmd::ListItem * pSeq = pItem; pSeq && pSeq->Item.m_ulSequenceNumber == ulCurSeqNumber; pSeqPrev = pSeq, pSeq = pSeq->pNextListItem )
		{
			AkLECmd & rCmd = pSeq->Item;
			if( rCmd.m_bSourceConnected || rCmd.m_eState > LEStatePlayPause || rCmd.m_pCtx->m_pCbx->m_eState == NodeStateStop )
			{
				continue;
			}
			const AKRESULT eResult = CAkLEngine::VPLTryConnectSource( rCmd.m_pCtx, rCmd.m_pCtx->m_pCbx );
			if( eResult == AK_Fail )
			{
				m_listCmd.RemoveItem( pSeq, pSeqPrev );
				goto StartOverProcCmds;
			}
			if( eResult == AK_FormatNotReady )
			{
				bDoSkipSeqNumber = true;
			}
			else
			{
				rCmd.m_bSourceConnected = true;
			}
		}
		if( bDoSkipSeqNumber )
		{
			do
			{
				pPrev = pItem;
				pItem = pItem->pNextListItem;
			}
			while( pItem && pItem->Item.m_ulSequenceNumber == ulCurSeqNumber );
			continue;
		}
		AkLECmd & rCmd = pItem->Item;
		CAkVPLSrcCbxNode * pCbx;
		if( rCmd.m_bSourceConnected )
		{
			pCbx = rCmd.m_pCtx->m_pCbx;
		}
		else
		{
			pCbx = CAkLEngine::ResolveCommandVPL( rCmd );
			if( !pCbx )
			{
				AkListCmd::ListItem * pNext = pItem->pNextListItem;
				m_listCmd.RemoveItem( pItem, pPrev );
				pItem = pNext;
				continue;
			}
		}
		if( rCmd.m_bSourceConnected )
		{
			switch( rCmd.m_eState )
			{
			case LEStatePlay:
				pCbx->Start();
				break;
			case LEStatePlayPause:
				pCbx->Start();
				pCbx->Pause();
				break;
			case LEStateStop:
				pCbx->Stop();
				break;
			case LEStatePause:
				pCbx->Pause();
				break;
			case LEStateResume:
				pCbx->Resume();
				break;
			case LEStateStopLooping:
				pCbx->StopLooping( rCmd.m_pCtx );
				break;
			case LEStateSeek:
				pCbx->SeekSource();
				break;
			default:
				break;
			}
		}
		else if( rCmd.m_eState == LEStateStop )
		{
			pCbx->Stop();
		}
		else
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		AkListCmd::ListItem * pNext = pItem->pNextListItem;
		m_listCmd.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
}

/** 2012 0x93ed60. */
void CAkLEngineCmds::ProcessAllCommands()
{
	if( m_bProcessPlayCmdsNeeded )
	{
		ProcessPlayCommands();
	}
	ProcessPendingCommands();
}

/** 2012 0x93ea50: sources waiting for their stream; stopped ones are destroyed with their commands, the others' frame
    offsets advance by one frame (scaled to the motion rate for feedback). */
void CAkLEngineCmds::ProcessDisconnectedSources( AkUInt32 in_uFrames )
{
	CAkVPLSrcCbxNode * pPrev = NULL;
	CAkVPLSrcCbxNode * pCbx = m_listSrcsNotConnected.First();
	while( pCbx )
	{
		CAkVPLSrcCbxNode * pNext = pCbx->pNextItem;
		if( pCbx->m_eState == NodeStateStop )
		{
			DequeuePBI( pCbx->GetContext() );
			if( pCbx == m_listSrcsNotConnected.m_pFirst )
			{
				m_listSrcsNotConnected.m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextItem = pNext;
			}
			if( pCbx == m_listSrcsNotConnected.m_pLast )
			{
				m_listSrcsNotConnected.m_pLast = pPrev;
			}
			--m_listSrcsNotConnected.m_ulNumListItems;
			CAkLEngine::VPLDestroySource( pCbx );
		}
		else
		{
			CAkPBI * pCtx = pCbx->GetContext();
			AkUInt32 uFrames = in_uFrames;
			if( pCtx->m_bTargetIsFeedback )
			{
				uFrames = ( uFrames * (AkUInt16)( 384000 / AkAudioLibSettings::g_pipelineCoreFrequency ) ) >> 10;
			}
			if( pCtx->m_iFrameOffset >= 0 )
			{
				pCtx->m_iFrameOffset -= uFrames;
			}
			pPrev = pCbx;
		}
		pCbx = pNext;
	}
}

/** 2012 0x93ebc0. */
void CAkLEngineCmds::DestroyDisconnectedSources()
{
	while( CAkVPLSrcCbxNode * pCbx = m_listSrcsNotConnected.RemoveFirst() )
	{
		CAkLEngine::VPLDestroySource( pCbx );
	}
}

/** 2012 0x91c770. */
void CAkLEngineCmds::AddDisconnectedSource( CAkVPLSrcCbxNode * in_pCbx )
{
	m_listSrcsNotConnected.AddLast( in_pCbx );
}
