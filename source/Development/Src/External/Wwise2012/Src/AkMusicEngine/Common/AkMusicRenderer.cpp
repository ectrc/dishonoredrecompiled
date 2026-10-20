/*=============================================================================
	AkMusicRenderer.cpp - CAkMusicRenderer and the AK::MusicEngine API (akmusicrenderer.cpp, 2012 0x961330..0x962110,
	2013 0x95db20..0x95e9d0).

	DISHONORED(port): agent IH4 fork M (wave 26). AK::MusicEngine::Init installs, as retail, the renderer's
	PerformNextFrameBehavior as a behavioural extension of the audio manager (run once per audio frame under g_csMain),
	CAkMusicRenderer::SetState as the external state handler and AkMusicBank::LoadBankItem as the external bank handler.
	Deviations (named):
	  * AddChild: when the node refuses the activity count, the count is decremented (retail calls
	    IncrementActivityCount a second time on that path, slot +0x90 both times).
	  * Play: the monitor notification of a refused play (MONITOR_OBJECTNOTIF, an empty function in the retail
	    build) is not made.
	  * The silent backend's AkSilent::Engine::bMusicEngineInit (still cleared by AkSilentCommon.cpp) is no longer set:
	    nothing reads it.
	  * Destroy waits, as retail, until every top-level music context is gone (StopAll + RenderAudio first); a leaked
	    context would hang the term.
=============================================================================*/
#include "AkMusicRenderer.h"
#include "AkMusicNode.h"
#include "AkMusicPBI.h"
#include "AkMatrixSequencer.h"
#include <AkURenderer.h>
#include <AkPlayingMgr.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

/** 2012 0xbab1c0, 2013 0xb8f740 / 0xbab1d0 / 0xbab1f0: the dynamic initializers of m_listCtx, m_segmentInfoRepository, m_queuePendingStateChanges. */
CAkMusicRenderer *		CAkMusicRenderer::m_pMusicRenderer = NULL;
AkMusicSettings			CAkMusicRenderer::m_musicSettings;
AkListBare<CAkMatrixAwareCtx, CAkMusicRenderer::AkListBareNextTopLevelCtxSibling>	CAkMusicRenderer::m_listCtx;
CAkSegmentInfoRepository	CAkMusicRenderer::m_segmentInfoRepository;
CAkMusicRenderer::PendingStateChanges	CAkMusicRenderer::m_queuePendingStateChanges;
HANDLE					CAkMusicRenderer::m_hTermEvent = NULL;

namespace AK
{
	namespace MusicEngine
	{
		/** 2012 0x961330, 2013 0x95db20. */
		void GetDefaultInitSettings( AkMusicSettings & out_settings )
		{
			out_settings.fStreamingLookAheadRatio = 1.f;
		}

		/** 2012 0x962110, 2013 0x95e9d0. */
		AKRESULT Init( AkMusicSettings * in_pSettings )
		{
			if( !CAkMusicRenderer::Create( in_pSettings ) )
			{
				return AK_Fail;
			}
			AK::SoundEngine::AddBehavioralExtension( CAkMusicRenderer::PerformNextFrameBehavior );
			AK::SoundEngine::AddExternalStateHandler( CAkMusicRenderer::SetState );
			AK::SoundEngine::AddExternalBankHandler( AkMusicBank::LoadBankItem );
			AKDIAG( L"akdiag: MusicEngine::Init: music renderer, behavioural extension, state and bank handlers installed (look-ahead ratio %.2f)", CAkMusicRenderer::m_musicSettings.fStreamingLookAheadRatio );
			return AK_Success;
		}

		/** 2012 0x962010, 2013 0x95e8d0. */
		void Term()
		{
			if( CAkMusicRenderer::m_pMusicRenderer )
			{
				CAkMusicRenderer::m_pMusicRenderer->Destroy();
			}
		}

		/** 2012 0x961380, 2013 0x95db70. */
		AKRESULT GetPlayingSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_segmentInfo, bool in_bExtrapolate )
		{
			return CAkMusicRenderer::m_segmentInfoRepository.GetSegmentInfo( in_PlayingID, out_segmentInfo, in_bExtrapolate );
		}
	}
}

/** 2012 0x962020, 2013 0x95e8e0. */
CAkMusicRenderer * CAkMusicRenderer::Create( AkMusicSettings * in_pSettings )
{
	if( m_pMusicRenderer )
	{
		return m_pMusicRenderer;
	}
	CAkMusicRenderer * pRenderer = (CAkMusicRenderer *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicRenderer ) );
	if( pRenderer )
	{
		m_pMusicRenderer = pRenderer;
		if( m_pMusicRenderer->Init( in_pSettings ) == AK_Success )
		{
			return m_pMusicRenderer;
		}
		m_pMusicRenderer->Destroy();
	}
	m_pMusicRenderer = NULL;
	return NULL;
}

/** 2012 0x961a20, 2013 0x95e210. */
AKRESULT CAkMusicRenderer::Init( AkMusicSettings * in_pSettings )
{
	if( in_pSettings )
	{
		m_musicSettings = *in_pSettings;
	}
	else
	{
		AK::MusicEngine::GetDefaultInitSettings( m_musicSettings );
	}
	m_hTermEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
	if( !m_hTermEvent )
	{
		return AK_Fail;
	}
	m_queuePendingStateChanges.InitUnbounded();
	return AK_Success;
}

/** 2012 0x961ec0, 2013 0x95e780: stop everything, let the audio thread run the stops, wait for the last context. */
void CAkMusicRenderer::Destroy()
{
	AK::SoundEngine::StopAll();
	AK::SoundEngine::RenderAudio();
	while( m_listCtx.First() )
	{
		WaitForSingleObject( m_hTermEvent, INFINITE );
	}
	if( m_hTermEvent )
	{
		CloseHandle( m_hTermEvent );
	}
	m_hTermEvent = NULL;
	m_listCtx.RemoveAll();
	m_segmentInfoRepository.Term();
	m_queuePendingStateChanges.Term();
	m_pMusicRenderer = NULL;
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x9613a0, 2013 0x95db90: a music PBI for one clip of a segment context, through the same limits and play
	counts as CAkURenderer::Play. */
AKRESULT CAkMusicRenderer::Play( CAkMusicCtx * io_pParentCtx, CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, UserParams & in_rUserparams, const AkTrackSrc * in_pSrcInfo, AkUInt32 in_uSourceOffset, AkUInt32 in_uFrameOffset, CAkMusicPBI *& out_pPBI )
{
	if( !in_pSound )
	{
		return AK_InvalidParameter;
	}
	AkReal32 fMaxRadius;
	const PriorityInfoCurrent priority = CAkURenderer::_CalcInitialPriority( in_pSound, in_pGameObj, fMaxRadius );
	const AKRESULT eValidateLimitsResult = CAkURenderer::ValidateLimits( priority.currentPriority.priority );
	if( eValidateLimitsResult == AK_Fail )
	{
		return AK_PartialSuccess;
	}
	CounterParameters counterParams;
	counterParams.fPriority = priority.currentPriority.priority;
	counterParams.pGameObj = in_pGameObj;
	const AKRESULT eIncrementResult = in_pSound->IncrementPlayCount( counterParams );
	bool bAllowedToPlay = eIncrementResult != AK_Fail;
	if( eIncrementResult == AK_MustBeVirtualized || eValidateLimitsResult == AK_MustBeVirtualized )
	{
		AkVirtualQueueBehavior _unused;
		bAllowedToPlay = CAkURenderer::GetVirtualBehaviorAction( in_pSound->GetVirtualBehavior( _unused ) );
	}
	AKRESULT eResult = AK_PartialSuccess;
	if( bAllowedToPlay )
	{
		AkPathInfo pathInfo;
		pathInfo.pPBPath = NULL;
		pathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
		PlayHistory playHistory;
		playHistory.HistArray.uiArraySize = 0;
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicPBI ) );
		out_pPBI = pMem ? AkPlacementNew( pMem ) CAkMusicPBI( io_pParentCtx, in_pSound, in_pSource, in_pGameObj, in_rUserparams, in_pSrcInfo, playHistory, 0, priority, in_uSourceOffset, counterParams.pAMLimiter, counterParams.pBusLimiter ) : NULL;
		if( out_pPBI )
		{
			if( out_pPBI->Init( &pathInfo ) == AK_Success )
			{
				out_pPBI->m_fMaxDistance = fMaxRadius;
				out_pPBI->m_iFrameOffset = in_uFrameOffset;
				Play( out_pPBI, in_transParams );
				return AK_Success;
			}
			out_pPBI->Term( true );
			out_pPBI->~CAkMusicPBI();
			AK::MemoryMgr::Free( g_DefaultPoolId, out_pPBI );
			out_pPBI = NULL;
			return AK_Fail;
		}
		eResult = AK_Fail;
	}
	CounterParameters counterParamsError;
	counterParamsError.pGameObj = in_pGameObj;
	in_pSound->DecrementPlayCount( counterParamsError );
	return eResult;
}

/** 2012 0x961350, 2013 0x95db40. */
void CAkMusicRenderer::Play( CAkMusicPBI * in_pContext, TransParams & in_transParams )
{
	CAkURenderer::EnqueueContext( in_pContext );
	in_pContext->_InitPlay();
	in_pContext->_Play( in_transParams, false, true );
}

/** 2012 0x961650, 2013 0x95de40. */
void CAkMusicRenderer::Stop( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, AkPlayingID in_playingID )
{
	CAkMatrixAwareCtx * pCtx = m_listCtx.First();
	while( pCtx )
	{
		CAkMatrixAwareCtx * pThis = pCtx;
		pCtx = pCtx->pNextTopLevelSibling;
		if( pThis->Node() == in_pNode
			&& ( !in_pGameObj || in_pGameObj == pThis->Sequencer()->m_pGameObj )
			&& ( !in_playingID || in_playingID == pThis->Sequencer()->m_UserParams.m_PlayingID ) )
		{
			pThis->_Stop( in_transParams, (AkUInt32)-1 );
		}
	}
}

/** 2012 0x9616b0, 2013 0x95dea0. */
void CAkMusicRenderer::Pause( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, AkPlayingID in_playingID )
{
	for( CAkMatrixAwareCtx * pCtx = m_listCtx.First(); pCtx; pCtx = pCtx->pNextTopLevelSibling )
	{
		if( pCtx->Node() == in_pNode
			&& ( !in_pGameObj || in_pGameObj == pCtx->Sequencer()->m_pGameObj )
			&& ( !in_playingID || in_playingID == pCtx->Sequencer()->m_UserParams.m_PlayingID ) )
		{
			pCtx->_Pause( in_transParams );
		}
	}
}

/** 2012 0x961710, 2013 0x95df00. */
void CAkMusicRenderer::Resume( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, bool in_bMasterResume, AkPlayingID in_playingID )
{
	for( CAkMatrixAwareCtx * pCtx = m_listCtx.First(); pCtx; pCtx = pCtx->pNextTopLevelSibling )
	{
		if( pCtx->Node() == in_pNode
			&& ( !in_pGameObj || in_pGameObj == pCtx->Sequencer()->m_pGameObj )
			&& ( !in_playingID || in_playingID == pCtx->Sequencer()->m_UserParams.m_PlayingID ) )
		{
			pCtx->_Resume( in_transParams, in_bMasterResume );
		}
	}
}

/** 2012 0x961780, 2013 0x95df70. */
void CAkMusicRenderer::SeekTimeAbsolute( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, AkTimeMs in_iSeekTime, bool in_bSnapToCue )
{
	CAkMatrixAwareCtx * pCtx = m_listCtx.First();
	while( pCtx )
	{
		CAkMatrixAwareCtx * pThis = pCtx;
		pCtx = pCtx->pNextTopLevelSibling;
		if( pThis->Node() == in_pNode
			&& ( !in_pGameObj || pThis->Sequencer()->m_pGameObj == in_pGameObj )
			&& pThis->SeekTimeAbsolute( in_iSeekTime, in_bSnapToCue ) == AK_Success )
		{
			g_pPlayingMgr->NotifyMusicPlayStarted( pThis->Sequencer()->m_UserParams.m_PlayingID );
		}
	}
}

/** 2012 0x9617f0, 2013 0x95dfe0. */
void CAkMusicRenderer::SeekPercent( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, AkReal32 in_fSeekPercent, bool in_bSnapToCue )
{
	CAkMatrixAwareCtx * pCtx = m_listCtx.First();
	while( pCtx )
	{
		CAkMatrixAwareCtx * pThis = pCtx;
		pCtx = pCtx->pNextTopLevelSibling;
		if( pThis->Node() == in_pNode
			&& ( !in_pGameObj || pThis->Sequencer()->m_pGameObj == in_pGameObj )
			&& pThis->SeekPercent( in_fSeekPercent, in_bSnapToCue ) == AK_Success )
		{
			g_pPlayingMgr->NotifyMusicPlayStarted( pThis->Sequencer()->m_UserParams.m_PlayingID );
		}
	}
}

/** 2012 0x961aa0, 2013 0x95e290: a new top-level context gets its sequencer (the clock), is registered with the playing
	manager under its playing id, and AK_MusicPlayStarted is notified. */
AKRESULT CAkMusicRenderer::AddChild( CAkMatrixAwareCtx * in_pMusicCtx, UserParams & in_rUserparams, CAkRegisteredObj * in_pGameObj )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMatrixSequencer ) );
	if( !pMem )
	{
		return AK_Fail;
	}
	CAkMatrixSequencer * pSequencer = AkPlacementNew( pMem ) CAkMatrixSequencer( in_pMusicCtx, in_rUserparams, in_pGameObj );
	CAkMusicNode * pNode = in_pMusicCtx->Node();
	if( !pNode || !pNode->IncrementActivityCount( 3 ) )
	{
		if( pNode )
		{
			pNode->DecrementActivityCount( 3 );
		}
		pSequencer->~CAkMatrixSequencer();
		AK::MemoryMgr::Free( g_DefaultPoolId, pSequencer );
		return AK_Fail;
	}
	m_listCtx.AddFirst( in_pMusicCtx );
	in_pMusicCtx->SetSequencer( pSequencer );
	const AkPlayingID playingID = in_rUserparams.m_PlayingID;
	if( !playingID )
	{
		return AK_Fail;
	}
	AkUInt32 uCallbackFlags = 0;
	const AKRESULT eResult = g_pPlayingMgr->SetPBI( playingID, in_pMusicCtx, &uCallbackFlags );
	in_pMusicCtx->SetRegisteredNotif( uCallbackFlags );
	if( ( uCallbackFlags & AK_EnableGetMusicPlayPosition ) && m_segmentInfoRepository.CreateEntry( playingID ) != AK_Success )
	{
		in_pMusicCtx->SetRegisteredNotif( uCallbackFlags & ~AK_EnableGetMusicPlayPosition );
	}
	g_pPlayingMgr->NotifyMusicPlayStarted( playingID );
	return eResult;
}

/** 2012 0x961cb0, 2013 0x95e4f0: the context's last reference is gone. */
void CAkMusicRenderer::RemoveChild( CAkMatrixAwareCtx * in_pMusicCtx )
{
	m_listCtx.Remove( in_pMusicCtx );
	CAkMatrixSequencer * pSequencer = in_pMusicCtx->Sequencer();
	if( pSequencer )
	{
		const AkPlayingID playingID = pSequencer->m_UserParams.m_PlayingID;
		if( playingID )
		{
			if( in_pMusicCtx->GetRegisteredNotif() & AK_EnableGetMusicPlayPosition )
			{
				m_segmentInfoRepository.RemoveEntry( playingID );
			}
			g_pPlayingMgr->Remove( playingID, in_pMusicCtx );
			if( in_pMusicCtx->Node() )
			{
				in_pMusicCtx->Node()->DecrementActivityCount( 3 );
			}
		}
		pSequencer->~CAkMatrixSequencer();
		AK::MemoryMgr::Free( g_DefaultPoolId, pSequencer );
	}
	SetEvent( m_hTermEvent );
}

/** 2012 0x961860, 2013 0x95e050: once per audio frame, every top-level context's sequencer runs one frame. */
void CAkMusicRenderer::PerformNextFrameBehavior( bool )
{
	CAkMatrixAwareCtx * pCtx = m_listCtx.First();
	while( pCtx )
	{
		CAkMatrixAwareCtx * pThis = pCtx;
		pCtx = pCtx->pNextTopLevelSibling;
		CAkMatrixSequencer * pSequencer = pThis->Sequencer();
		AkSegmentInfo segmentInfo;
		if( ( pThis->GetRegisteredNotif() & AK_EnableGetMusicPlayPosition ) && pThis->GetPlayingSegmentInfo( segmentInfo ) == AK_Success )
		{
			m_segmentInfoRepository.UpdateSegmentInfo( pSequencer->m_UserParams.m_PlayingID, segmentInfo );
		}
		pSequencer->Execute( AK_NUM_VOICE_REFILL_FRAMES );
	}
}

/** 2012 0x9618c0, 2013 0x95e0b0: the playing, unpaused context whose sync to this state group comes first; the delay
	less one audio frame (the state change itself takes effect one frame later). */
AkInt64 CAkMusicRenderer::GetDelayedStateChangeData( AkStateGroupID in_stateGroupID, CAkMatrixAwareCtx *& out_pChosenCtx, AkInt64 & out_iChosenRelativeSyncTime, AkUInt32 & out_uChosenSegmentLookAhead )
{
	AkInt64 iEarliestAbsoluteDelay = 0;
	out_pChosenCtx = NULL;
	for( CAkMatrixAwareCtx * pCtx = m_listCtx.First(); pCtx; pCtx = pCtx->pNextTopLevelSibling )
	{
		if( ( pCtx->m_eStatus & CtxStatusPlaying ) && !pCtx->IsPaused() )
		{
			AkUInt32 uSegmentLookAhead;
			AkInt64 iRelativeSyncTime;
			const AkInt64 iAbsoluteDelay = pCtx->Sequencer()->QueryStateChangeDelay( in_stateGroupID, uSegmentLookAhead, iRelativeSyncTime );
			if( !out_pChosenCtx || iAbsoluteDelay < iEarliestAbsoluteDelay )
			{
				out_iChosenRelativeSyncTime = iRelativeSyncTime;
				iEarliestAbsoluteDelay = iAbsoluteDelay;
				out_uChosenSegmentLookAhead = uSegmentLookAhead;
				out_pChosenCtx = pCtx;
			}
		}
	}
	return iEarliestAbsoluteDelay - AK_NUM_VOICE_REFILL_FRAMES;
}

/** 2012 0x961d50, 2013 0x95e590: the external state handler. True takes the change: it is posted later from the
	sequencer at the sync point (skipping this handler). */
bool CAkMusicRenderer::SetState( AkStateGroupID in_stateGroupID, AkStateID in_stateID )
{
	CAkMatrixAwareCtx * pChosenCtx = NULL;
	AkInt64 iChosenRelativeSyncTime;
	AkUInt32 uChosenSegmentLookAhead;
	if( GetDelayedStateChangeData( in_stateGroupID, pChosenCtx, iChosenRelativeSyncTime, uChosenSegmentLookAhead ) <= 0 )
	{
		return false;
	}
	AkStateChangeRecord * pRecord = AddFirstPendingStateChange();
	if( !pRecord )
	{
		InvalidateOlderPendingStateChanges( m_queuePendingStateChanges.m_pFirst, in_stateGroupID );
		return false;
	}
	if( pChosenCtx->Sequencer()->ProcessDelayedStateChange( pRecord, uChosenSegmentLookAhead, iChosenRelativeSyncTime ) != AK_Success )
	{
		if( m_queuePendingStateChanges.m_pFirst )
		{
			m_queuePendingStateChanges.RemoveItem( m_queuePendingStateChanges.m_pFirst, NULL );
		}
		return false;
	}
	pRecord->stateGroupID = in_stateGroupID;
	pRecord->stateID = in_stateID;
	pRecord->bWasPosted = 0;
	pRecord->bIsReferenced = 1;
	AKDIAG( L"akdiag: music: state group 0x%08x -> 0x%08x delayed %I64d samples to the sync point", in_stateGroupID, in_stateID, iChosenRelativeSyncTime );
	return true;
}

/** 2012 0x961c50: CAkList2::AddFirst of the pending queue (retail's own template instance). */
CAkMusicRenderer::AkStateChangeRecord * CAkMusicRenderer::AddFirstPendingStateChange()
{
	PendingStateChanges & queue = m_queuePendingStateChanges;
	if( !queue.m_pFree )
	{
		if( queue.m_ulNumListItems >= queue.m_ulMaxNumListItems )
		{
			return NULL;
		}
		PendingStateChanges::ListItem * pNew = (PendingStateChanges::ListItem *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( PendingStateChanges::ListItem ) );
		if( !pNew )
		{
			return NULL;
		}
		pNew->pNextListItem = queue.m_pFree;
		queue.m_pFree = pNew;
	}
	PendingStateChanges::ListItem * pItem = queue.m_pFree;
	if( !queue.m_pFirst )
	{
		queue.m_pLast = pItem;
	}
	queue.m_pFree = pItem->pNextListItem;
	pItem->pNextListItem = queue.m_pFirst;
	++queue.m_ulNumListItems;
	queue.m_pFirst = pItem;
	return &pItem->Item;
}

/** 2012 0x961950, 2013 0x95e140. */
void CAkMusicRenderer::FindPendingStateChange( void * in_pCookie, PendingStateChanges::ListItem *& out_pItem, PendingStateChanges::ListItem *& out_pPrevItem )
{
	out_pItem = m_queuePendingStateChanges.m_pFirst;
	out_pPrevItem = NULL;
	while( out_pItem && &out_pItem->Item != in_pCookie )
	{
		out_pPrevItem = out_pItem;
		out_pItem = out_pItem->pNextListItem;
	}
}

/** 2012 0x961990, 2013 0x95e180: from in_pItem on, every change of the group is marked as posted (superseded). */
void CAkMusicRenderer::InvalidateOlderPendingStateChanges( PendingStateChanges::ListItem * in_pItem, AkStateGroupID in_stateGroupID )
{
	for( ; in_pItem; in_pItem = in_pItem->pNextListItem )
	{
		if( in_pItem->Item.stateGroupID == in_stateGroupID )
		{
			in_pItem->Item.bWasPosted = 1;
		}
	}
}

/** 2012 0x961f50, 2013 0x95e810: the sequencer reached the sync point. */
void CAkMusicRenderer::PerformDelayedStateChange( void * in_pCookie )
{
	PendingStateChanges::ListItem * pItem;
	PendingStateChanges::ListItem * pPrevItem;
	FindPendingStateChange( in_pCookie, pItem, pPrevItem );
	pItem->Item.bIsReferenced = 0;
	if( !pItem->Item.bWasPosted )
	{
		pItem->Item.bWasPosted = 1;
		const AkStateGroupID stateGroupID = pItem->Item.stateGroupID;
		AKDIAG( L"akdiag: music: delayed state group 0x%08x -> 0x%08x posted at its sync point", stateGroupID, pItem->Item.stateID );
		AK::SoundEngine::SetState( stateGroupID, pItem->Item.stateID, false, true );
		InvalidateOlderPendingStateChanges( pItem->pNextListItem, stateGroupID );
	}
	CleanPendingStateChanges();
}

/** 2012 0x962070, 2013 0x95e930: the scheduled sync was cancelled; find a new one or post the change now. */
void CAkMusicRenderer::RescheduleDelayedStateChange( void * in_pCookie )
{
	PendingStateChanges::ListItem * pItem;
	PendingStateChanges::ListItem * pPrevItem;
	FindPendingStateChange( in_pCookie, pItem, pPrevItem );
	if( pItem->Item.bWasPosted )
	{
		pItem->Item.bIsReferenced = 0;
		CleanPendingStateChanges();
		return;
	}
	const AkStateGroupID stateGroupID = pItem->Item.stateGroupID;
	CAkMatrixAwareCtx * pChosenCtx = NULL;
	AkInt64 iChosenRelativeSyncTime;
	AkUInt32 uChosenSegmentLookAhead;
	if( GetDelayedStateChangeData( stateGroupID, pChosenCtx, iChosenRelativeSyncTime, uChosenSegmentLookAhead ) <= 0
		|| pChosenCtx->Sequencer()->ProcessDelayedStateChange( in_pCookie, uChosenSegmentLookAhead, iChosenRelativeSyncTime ) != AK_Success )
	{
		CancelDelayedStateChange( stateGroupID, pItem );
	}
}

/** 2012 0x961fc0, 2013 0x95e880: post the change now and supersede the group's older ones. */
void CAkMusicRenderer::CancelDelayedStateChange( AkStateGroupID in_stateGroupID, PendingStateChanges::ListItem * in_pItem )
{
	AK::SoundEngine::SetState( in_stateGroupID, in_pItem->Item.stateID, false, true );
	in_pItem->Item.bWasPosted = 1;
	in_pItem->Item.bIsReferenced = 0;
	InvalidateOlderPendingStateChanges( in_pItem, in_stateGroupID );
	CleanPendingStateChanges();
}

/** 2012 0x961e20, 2013 0x95e660: drop every record no sequencer action refers to any more. */
void CAkMusicRenderer::CleanPendingStateChanges()
{
	PendingStateChanges::ListItem * pPrevItem = NULL;
	PendingStateChanges::ListItem * pItem = m_queuePendingStateChanges.m_pFirst;
	while( pItem )
	{
		PendingStateChanges::ListItem * pNextItem = pItem->pNextListItem;
		if( pItem->Item.bIsReferenced )
		{
			pPrevItem = pItem;
		}
		else
		{
			m_queuePendingStateChanges.RemoveItem( pItem, pPrevItem );
		}
		pItem = pNextItem;
	}
}
