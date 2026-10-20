/*=============================================================================
	AkMatrixSequencer.cpp - CAkScheduledItem and CAkMatrixSequencer (akmatrixsequencer.cpp, 2012 0x962510..0x963b20).

	DISHONORED(port): agent IH4 (wave 26).
	Deviations (named):
	  * FreeStingerRecord is the CAkSmartPtr<CAkMatrixAwareCtx> release + free retail inlines in ScheduleStingerForPlayback,
	    ProcessStingers and RemoveAllPendingStingers.
	  * -akdiag: every stinger scheduled and every delayed state change queued is logged.
=============================================================================*/
#include "AkMatrixSequencer.h"
#include "AkScheduleWindow.h"
#include "AkSegmentCtx.h"
#include "AkMusicSegment.h"
#include "AkMusicRenderer.h"
#include "AkPlayingMgr.h"
#include <AkRegistryMgr.h>
#include <DishonoredWwiseSilent.h>

// ------------------------------------------------------------------------------------------------------------
// CAkScheduledItem
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x962a80, 2013 0x95f2c0: the segment context is referenced and told its owner. */
CAkScheduledItem::CAkScheduledItem( AkUInt64 in_iLocalTime, CAkSegmentCtx * in_pSegment )
	: pNextItem( NULL )
	, m_iLocalTime( in_iLocalTime )
	, m_pSegment( in_pSegment )
{
	m_listAssociatedActions.m_pFirst = NULL;
	if( in_pSegment )
	{
		in_pSegment->AddRef();
	}
	m_cmdPlay.iRelativeTime = 0;
	m_cmdPlay.fadeParams.transitionTime = 0;
	m_cmdPlay.fadeParams.eFadeCurve = AkCurveInterpolation_Linear;
	m_cmdPlay.fadeParams.iFadeOffset = 0;
	m_cmdStop.iRelativeTime = 0;
	m_cmdStop.transParams.TransitionTime = 0;
	m_cmdStop.transParams.eFadeCurve = AkCurveInterpolation_Linear;
	m_bCmdPlayPending = 0;
	m_bCmdStopPending = 0;
	m_bWasPlaybackSkipped = 0;
	if( m_pSegment )
	{
		m_pSegment->m_pOwner = this;
	}
}

/** 2012 0x962510, 2013 0x95ed50. */
void CAkScheduledItem::SetFadeIn( AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, AkInt32 in_iFadeOffset )
{
	m_bCmdPlayPending = 1;
	m_cmdPlay.fadeParams.transitionTime = in_iTransDuration;
	m_cmdPlay.fadeParams.eFadeCurve = in_eFadeCurve;
	m_cmdPlay.fadeParams.iFadeOffset = in_iFadeOffset;
}

/** 2012 0x962530, 2013 0x95ed70: a stop before the play command is moved onto it, its fade shortened by the difference. */
void CAkScheduledItem::AttachStopCmd( AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, AkInt32 in_iRelativeTime )
{
	AkInt32 iRelativeTime = in_iRelativeTime;
	const AkInt32 iPlayTime = m_cmdPlay.iRelativeTime;
	m_cmdStop.transParams.TransitionTime = in_iTransDuration;
	if( in_iRelativeTime < iPlayTime )
	{
		m_cmdStop.transParams.TransitionTime = in_iTransDuration + AkTimeConv::SamplesToMilliseconds( in_iRelativeTime - iPlayTime );
		iRelativeTime = iPlayTime;
	}
	m_bCmdStopPending = 1;
	m_cmdStop.transParams.eFadeCurve = in_eFadeCurve;
	m_cmdStop.iRelativeTime = iRelativeTime;
}

/** 2012 0x962580, 2013 0x95edc0: the segment's position and durations in ms (zeros for a silent item). */
AKRESULT CAkScheduledItem::GetInfo( AkInt32 in_iSegmentPosition, AkSegmentInfo & out_uSegmentInfo )
{
	if( !m_pSegment )
	{
		out_uSegmentInfo.iPreEntryDuration = 0;
		out_uSegmentInfo.iActiveDuration = 0;
		out_uSegmentInfo.iPostExitDuration = 0;
		out_uSegmentInfo.iCurrentPosition = 0;
		out_uSegmentInfo.iRemainingLookAheadTime = 0;
		return AK_Success;
	}
	CAkMusicSegment * pNode = m_pSegment->m_pSegmentNode;
	out_uSegmentInfo.iPreEntryDuration = AkTimeConv::SamplesToMilliseconds( pNode->PreEntryDuration() );
	out_uSegmentInfo.iActiveDuration = AkTimeConv::SamplesToMilliseconds( (AkInt32)pNode->ActiveDuration() );
	out_uSegmentInfo.iPostExitDuration = AkTimeConv::SamplesToMilliseconds( (AkInt32)pNode->PostExitDuration() );
	out_uSegmentInfo.iCurrentPosition = AkTimeConv::SamplesToMilliseconds( in_iSegmentPosition );
	if( m_pSegment->m_iAudibleTime - in_iSegmentPosition <= 0 )
	{
		out_uSegmentInfo.iRemainingLookAheadTime = 0;
	}
	else
	{
		out_uSegmentInfo.iRemainingLookAheadTime = AkTimeConv::SamplesToMilliseconds( m_pSegment->m_iAudibleTime - in_iSegmentPosition );
	}
	return AK_Success;
}

/**
 * 2012 0x962630: the segment context is prepared to be audible from in_iStartPosition (a fade-in starts no later than
 * its offset); the play command is placed its look-ahead before the sync position. Returns that look-ahead.
 */
AkInt32 CAkScheduledItem::Prepare( AkUInt32 in_uSyncPosition, AkInt32 in_iStartPosition, const AkMusicFade & in_fade )
{
	AkInt32 iLookAhead = 0;
	if( m_pSegment )
	{
		AkInt32 iStartPosition = in_iStartPosition;
		if( in_fade.transitionTime > 0 && in_iStartPosition < in_fade.iFadeOffset )
		{
			iStartPosition = in_fade.iFadeOffset;
		}
		iLookAhead = (AkInt32)in_uSyncPosition - iStartPosition + m_pSegment->Prepare( iStartPosition );
	}
	m_bCmdPlayPending = 1;
	m_cmdPlay.iRelativeTime = (AkInt32)in_uSyncPosition - iLookAhead;
	m_cmdPlay.fadeParams.transitionTime = 0;
	return iLookAhead;
}

/** 2012 0x962680, 2013 0x95eec0: an unfaded stop moves to the exit cue, or past the post-exit. */
void CAkScheduledItem::ForcePostExit( bool in_bPlayPostExit )
{
	if( !m_pSegment || m_cmdStop.transParams.TransitionTime )
	{
		return;
	}
	CAkMusicSegment * pNode = m_pSegment->m_pSegmentNode;
	if( in_bPlayPostExit )
	{
		m_cmdStop.iRelativeTime = (AkInt32)( pNode->ActiveDuration() + pNode->PostExitDuration() );
	}
	else
	{
		m_cmdStop.iRelativeTime = (AkInt32)pNode->ActiveDuration();
	}
}

/** 2012 0x9626d0, 2013 0x95ef10: the segment fades out over as long as it has been audible. */
void CAkScheduledItem::_CancelPlayback( AkInt32 in_iCurrentTime )
{
	CAkSegmentCtx * pSegment = m_pSegment;
	AkInt32 iElapsed = in_iCurrentTime - pSegment->m_iAudibleTime;
	if( iElapsed <= 0 )
	{
		iElapsed = 0;
	}
	const AkReal64 v = (AkReal64)iElapsed * 1000.0 / (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency;
	TransParams revTransParams;
	revTransParams.eFadeCurve = AkCurveInterpolation_Linear;
	revTransParams.TransitionTime = (AkInt32)( v + ( v <= 0.0 ? -0.5 : 0.5 ) );
	pSegment->_Stop( revTransParams, (AkUInt32)-1 );
	m_bWasPlaybackSkipped = 1;
}

/** 2012 0x962760, 2013 0x95efa0. */
bool CAkScheduledItem::CanDestroy()
{
	return !m_pSegment && !m_bCmdStopPending && !m_listAssociatedActions.m_pFirst;
}

/** 2012 0x962780, 2013 0x95efc0: the music sync callbacks for the frame (user cues, entry, bars, beats, grid, exit). */
void CAkScheduledItem::NotifyMusicCallbacks( AkInt32 in_iSegmentPosition, AkUInt32 in_uFrameDuration, AkUInt32 in_uNotifFlags, AkPlayingID in_playingID ) const
{
	if( !in_uNotifFlags )
	{
		return;
	}
	CAkMusicSegment * pNode = m_pSegment->m_pSegmentNode;
	const AkMusicGrid & rGrid = pNode->GetMusicGrid();
	if( in_uNotifFlags & AK_MusicSyncUserCue )
	{
		pNode->NotifyUserCuesInRange( in_playingID, rGrid, in_iSegmentPosition, in_uFrameDuration );
	}
	if( ( in_uNotifFlags & AK_MusicSyncEntry ) && in_iSegmentPosition <= 0 && (AkInt32)( in_uFrameDuration + in_iSegmentPosition ) > 0 )
	{
		g_pPlayingMgr->NotifyMusic( in_playingID, AK_MusicSyncEntry, rGrid );
	}
	AkUInt32 uNumBars, uNumBeats, uNumGrids;
	pNode->GetNumMusicGridInRange( in_iSegmentPosition, in_uFrameDuration, uNumBars, uNumBeats, uNumGrids );
	if( in_uNotifFlags & AK_MusicSyncBar )
	{
		for( AkUInt32 i = 0; i < uNumBars; ++i )
		{
			g_pPlayingMgr->NotifyMusic( in_playingID, AK_MusicSyncBar, rGrid );
		}
	}
	if( in_uNotifFlags & AK_MusicSyncBeat )
	{
		for( AkUInt32 i = 0; i < uNumBeats; ++i )
		{
			g_pPlayingMgr->NotifyMusic( in_playingID, AK_MusicSyncBeat, rGrid );
		}
	}
	if( in_uNotifFlags & AK_MusicSyncGrid )
	{
		for( AkUInt32 i = 0; i < uNumGrids; ++i )
		{
			g_pPlayingMgr->NotifyMusic( in_playingID, AK_MusicSyncGrid, rGrid );
		}
	}
	if( in_uNotifFlags & AK_MusicSyncExit )
	{
		const AkInt32 iActiveDuration = (AkInt32)m_pSegment->m_pSegmentNode->ActiveDuration();
		if( iActiveDuration >= in_iSegmentPosition && iActiveDuration < (AkInt32)( in_uFrameDuration + in_iSegmentPosition ) )
		{
			g_pPlayingMgr->NotifyMusic( in_playingID, AK_MusicSyncExit, rGrid );
		}
	}
}

/**
 * 2012 0x962cb0: the pending play and stop commands falling in this frame run (a play past an active cutoff is skipped
 * and a playing segment there is cancelled); the segment processes the frame; due attached actions fire.
 */
void CAkScheduledItem::Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, bool in_bSkipPlay )
{
	const AkInt64 iFrameStart = in_iCurrentTime - (AkInt64)m_iLocalTime;
	const AkInt64 iFrameEnd = iFrameStart + in_uNumSamples;
	if( m_pSegment )
	{
		if( m_bCmdPlayPending )
		{
			const AkInt32 iPlayTime = m_cmdPlay.iRelativeTime;
			if( (AkInt32)iFrameStart <= iPlayTime && iFrameEnd > iPlayTime )
			{
				if( in_bSkipPlay )
				{
					m_bWasPlaybackSkipped = 1;
				}
				else
				{
					m_pSegment->_Play( m_cmdPlay.fadeParams );
				}
				m_bCmdPlayPending = 0;
			}
		}
		else if( in_bSkipPlay && ( m_pSegment->m_eStatus & CtxStatusPlaying ) && !m_bWasPlaybackSkipped )
		{
			_CancelPlayback( (AkInt32)iFrameStart );
		}
		if( m_bCmdStopPending )
		{
			const AkInt32 iStopTime = m_cmdStop.iRelativeTime;
			if( iFrameEnd > iStopTime && (AkInt32)iFrameStart <= iStopTime )
			{
				m_pSegment->_Stop( m_cmdStop.transParams, (AkUInt32)( iStopTime - (AkInt32)iFrameStart ) );
				m_bCmdStopPending = 0;
			}
		}
		CAkSegmentCtx * pSegment = m_pSegment;
		if( pSegment && ( pSegment->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !pSegment->m_bIsPaused )
		{
			pSegment->Process( (AkInt32)iFrameStart, in_uNumSamples );
		}
	}
	AkAssociatedAction * pPrev = NULL;
	AkAssociatedAction * pAction = m_listAssociatedActions.m_pFirst;
	while( pAction )
	{
		if( iFrameEnd <= pAction->iRelativeTime )
		{
			pPrev = pAction;
			pAction = pAction->pNextLightItem;
			continue;
		}
		if( pAction->eActionType == AssocActionTypeState )
		{
			CAkMusicRenderer::m_pMusicRenderer->PerformDelayedStateChange( pAction->pStateChangeCookie );
		}
		AkAssociatedAction * pNext = pAction->pNextLightItem;
		if( pAction == m_listAssociatedActions.m_pFirst )
		{
			m_listAssociatedActions.m_pFirst = pNext;
		}
		else
		{
			pPrev->pNextLightItem = pNext;
		}
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
		pAction = pNext;
	}
}

/** 2012 0x962e10, 2013 0x95f650: pending commands drop; delayed state changes fire now; the segment context is released. */
void CAkScheduledItem::OnStopped()
{
	m_bCmdPlayPending = 0;
	m_bCmdStopPending = 0;
	while( AkAssociatedAction * pAction = m_listAssociatedActions.m_pFirst )
	{
		if( pAction->eActionType == AssocActionTypeState )
		{
			CAkMusicRenderer::m_pMusicRenderer->PerformDelayedStateChange( pAction->pStateChangeCookie );
		}
		m_listAssociatedActions.m_pFirst = pAction->pNextLightItem;
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
	}
	if( m_pSegment )
	{
		m_pSegment->m_pOwner = NULL;
		CAkSegmentCtx * pSegment = m_pSegment;
		m_pSegment = NULL;
		pSegment->Release();
	}
}

/** 2012 0x962ea0, 2013 0x95f6e0: a pause fires the delayed state changes at once (the stingers stay). */
void CAkScheduledItem::OnPaused()
{
	AkAssociatedAction * pPrev = NULL;
	AkAssociatedAction * pAction = m_listAssociatedActions.m_pFirst;
	while( pAction )
	{
		if( pAction->eActionType != AssocActionTypeState )
		{
			pPrev = pAction;
			pAction = pAction->pNextLightItem;
			continue;
		}
		CAkMusicRenderer::m_pMusicRenderer->PerformDelayedStateChange( pAction->pStateChangeCookie );
		AkAssociatedAction * pNext = pAction->pNextLightItem;
		if( pAction == m_listAssociatedActions.m_pFirst )
		{
			m_listAssociatedActions.m_pFirst = pNext;
		}
		else
		{
			pPrev->pNextLightItem = pNext;
		}
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
		pAction = pNext;
	}
}

/** 2012 0x962f00, 2013 0x95f740: a playing segment is cancelled with a fade; anything else just stops. */
void CAkScheduledItem::CancelPlayback( AkInt64 in_iCurrentTime )
{
	if( m_pSegment && ( m_pSegment->m_eStatus & CtxStatusPlaying ) && !m_bWasPlaybackSkipped )
	{
		_CancelPlayback( (AkInt32)in_iCurrentTime - (AkInt32)m_iLocalTime );
	}
	else
	{
		OnStopped();
	}
}

/** 2012 0x962f30, 2013 0x95f770: actions after the sync point (stingers at it too) move to the cancelled list. */
void CAkScheduledItem::PopAssociatedActionsToRescheduleAfterTransitionSyncPoint( AkListBareLight<AkAssociatedAction> & io_listCancelledActions, AkInt64 in_iMinActionTime )
{
	AkAssociatedAction * pPrev = NULL;
	AkAssociatedAction * pAction = m_listAssociatedActions.m_pFirst;
	while( pAction )
	{
		if( pAction->iRelativeTime > in_iMinActionTime || ( pAction->eActionType == AssocActionTypeStinger && pAction->iRelativeTime == in_iMinActionTime ) )
		{
			AkAssociatedAction * pNext = pAction->pNextLightItem;
			if( pAction == m_listAssociatedActions.m_pFirst )
			{
				m_listAssociatedActions.m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextLightItem = pNext;
			}
			io_listCancelledActions.AddFirst( pAction );
			pAction = pNext;
		}
		else
		{
			pPrev = pAction;
			pAction = pAction->pNextLightItem;
		}
	}
}

/** 2012 0x9630d0, 2013 0x95f910. */
void CAkScheduledItem::Destroy()
{
	m_listAssociatedActions.m_pFirst = NULL;
	if( m_pSegment )
	{
		m_pSegment->Release();
		m_pSegment = NULL;
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

// ------------------------------------------------------------------------------------------------------------
// CAkMatrixSequencer
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x962ad0, 2013 0x95f310: the sequencer holds the game object and listens to its triggers. */
CAkMatrixSequencer::CAkMatrixSequencer( CAkMatrixAwareCtx * in_pOwner, UserParams & in_rUserparams, CAkRegisteredObj * in_pGameObj )
	: m_uTime( 0 )
	, m_pOwner( in_pOwner )
	, m_UserParams( in_rUserparams )
	, m_pGameObj( in_pGameObj )
	, m_uCurTimeWindowSize( 0 )
{
	m_listPendingStingers.m_pFirst = NULL;
	++m_pGameObj->m_refCount;
	g_pStateMgr->RegisterTrigger( this, m_pGameObj );
}

/** 2012 0x9639c0, 2013 0x960200. */
CAkMatrixSequencer::~CAkMatrixSequencer()
{
	RemoveAllPendingStingers();
	g_pStateMgr->UnregisterTrigger( this );
	m_listPendingStingers.m_pFirst = NULL;
	if( m_pGameObj )
	{
		if( --m_pGameObj->m_refCount <= 0 )
		{
			m_pGameObj->~CAkRegisteredObj();
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pGameObj );
		}
		m_pGameObj = NULL;
	}
}

/** 2012 0x962570, 2013 0x95edb0: the AK_MusicSync* bits the playing id registered. */
AkUInt32 CAkMatrixSequencer::GetMusicSyncFlags()
{
	return m_pOwner->m_uRegisteredNotif & AK_MusicSyncAll;
}

/** 2012 0x9628f0, 2013 0x95f130: one audio frame of the top-level context (unless idle, stopped or paused), then the clock advances. */
void CAkMatrixSequencer::Execute( AkUInt32 in_uNumFrames )
{
	m_pOwner->AddRef();
	CAkMatrixAwareCtx * pOwner = m_pOwner;
	if( ( pOwner->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !pOwner->m_bIsPaused )
	{
		m_uCurTimeWindowSize = in_uNumFrames;
		AkCutoffInfo cutoffInfo;
		cutoffInfo.iCutoffTime = 0;
		cutoffInfo.bCutoff = false;
		pOwner->Process( (AkInt64)m_uTime, in_uNumFrames, cutoffInfo );
		m_uTime += m_uCurTimeWindowSize;
	}
	m_pOwner->Release();
}

/** 2012 0x962950, 2013 0x95f190: the callbacks of every window this frame touches. */
void CAkMatrixSequencer::ProcessMusicNotifications( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration )
{
	CAkScheduleWindow window( m_pOwner, false );
	if( !window.m_itScheduledItem.pItem )
	{
		return;
	}
	const AkUInt32 uFlags = m_pOwner->m_uRegisteredNotif & AK_MusicSyncAll;
	window.NotifyMusicCallbacks( in_iCurrentTime, in_uFrameDuration, uFlags, m_UserParams.m_PlayingID );
	while( !window.m_bIsDurationInfinite )
	{
		const AkInt64 iSegmentPosition = window.m_itScheduledItem.CtxTimeToSegmentPosition( in_iCurrentTime - window.ChainCtxTimeRelativeToLevel( 0 ) );
		const AkUInt64 uRemaining = iSegmentPosition >= (AkInt64)window.m_uWindowDuration ? 0 : window.m_uWindowDuration - iSegmentPosition;
		if( uRemaining >= in_uFrameDuration )
		{
			break;
		}
		m_pOwner->GetNextScheduleWindow( window, false );
		window.NotifyMusicCallbacks( in_iCurrentTime, in_uFrameDuration, uFlags, m_UserParams.m_PlayingID );
	}
}

/** 2012 0x962b60, 2013 0x95f3a0: a trigger's stinger does not repeat within its don't-repeat time. */
bool CAkMatrixSequencer::CanPlayStinger( AkTriggerID in_triggerID )
{
	for( AkStingerRecord * pRecord = m_listPendingStingers.m_pFirst; pRecord; pRecord = pRecord->pNextLightItem )
	{
		if( pRecord->triggerID == in_triggerID && pRecord->iSyncTime + pRecord->uDontRepeatTime >= (AkInt64)m_uTime - m_pOwner->m_iLocalTime )
		{
			return false;
		}
	}
	return true;
}

/** 2012 0x962bb0, 2013 0x95f3f0: a delayed state change is attached to the item in_uSegmentLookAhead windows ahead, at its sync time. */
AKRESULT CAkMatrixSequencer::ProcessDelayedStateChange( void * in_pCookie, AkUInt32 in_uSegmentLookAhead, AkInt64 in_iSyncTime )
{
	CAkScheduleWindow window( m_pOwner, false );
	if( !window.m_itScheduledItem.pItem )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = in_uSegmentLookAhead; i; --i )
	{
		m_pOwner->GetNextScheduleWindow( window, false );
	}
	AkAssociatedAction * pAction = (AkAssociatedAction *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkAssociatedAction ) );
	if( !pAction )
	{
		return AK_Fail;
	}
	pAction->eActionType = AssocActionTypeState;
	pAction->iRelativeTime = window.m_itScheduledItem.CtxTimeToSegmentPosition( in_iSyncTime - window.ChainCtxTimeRelativeToLevel( 0 ) );
	pAction->pStateChangeCookie = in_pCookie;
	window.m_itScheduledItem.pItem->m_listAssociatedActions.AddFirst( pAction );
	AKDIAG( L"akdiag: music state change scheduled: segment position %lld (sequencer time %llu, sync %lld, look-ahead %u)", pAction->iRelativeTime, m_uTime, in_iSyncTime, in_uSegmentLookAhead );
	return AK_Success;
}

/** The CAkSmartPtr release + free retail inlines for a stinger record. */
void CAkMatrixSequencer::FreeStingerRecord( AkStingerRecord * in_pRecord )
{
	CAkMatrixAwareCtx * pCtx = in_pRecord->pStingerCtx;
	in_pRecord->pStingerCtx = NULL;
	if( pCtx )
	{
		pCtx->Release();
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pRecord );
}

/** 2012 0x963110, 2013 0x95f950: the stinger's record (pending list) and its action on the window's item at the sync time. */
AKRESULT CAkMatrixSequencer::ScheduleStingerForPlayback( const CAkScheduleWindow & in_window, CAkMatrixAwareCtx * in_pStingerCtx, const CAkStinger * in_pStingerData, AkInt64 in_iSyncTime, AkInt32 in_iLookAheadDuration, bool in_bScheduledInCurrentSegment )
{
	AkStingerRecord * pRecord = (AkStingerRecord *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkStingerRecord ) );
	if( !pRecord )
	{
		return AK_Fail;
	}
	pRecord->pStingerCtx = NULL;
	AkAssociatedAction * pAction = (AkAssociatedAction *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkAssociatedAction ) );
	if( !pAction )
	{
		FreeStingerRecord( pRecord );
		return AK_Fail;
	}
	pRecord->triggerID = in_pStingerData->m_TriggerID;
	pRecord->segmentID = in_pStingerData->m_SegmentID;
	pRecord->iSyncTime = in_iSyncTime;
	pRecord->uDontRepeatTime = in_pStingerData->m_DontRepeatTime * ( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	pRecord->iLookAheadDuration = in_iLookAheadDuration;
	if( in_pStingerCtx )
	{
		in_pStingerCtx->AddRef();
		in_pStingerCtx->m_iLocalTime = in_iSyncTime;
		pRecord->pStingerCtx = in_pStingerCtx;
		CAkMusicSegment * pSegment = in_pStingerCtx->GetFirstSegmentNode();
		pRecord->iRelativeStopTime = pSegment ? (AkInt32)( pSegment->ActiveDuration() + pSegment->PostExitDuration() ) : 0;
	}
	pRecord->bCanBeRescheduled = in_bScheduledInCurrentSegment && in_pStingerData->m_numSegmentLookAhead;
	m_listPendingStingers.AddFirst( pRecord );
	pAction->eActionType = AssocActionTypeStinger;
	pAction->iRelativeTime = in_window.m_itScheduledItem.CtxTimeToSegmentPosition( in_iSyncTime - in_window.ChainCtxTimeRelativeToLevel( 0 ) );
	pAction->pStingerRecord = pRecord;
	in_window.m_itScheduledItem.pItem->m_listAssociatedActions.AddFirst( pAction );
	AKDIAG( L"akdiag: music stinger trigger 0x%08x segment 0x%08x scheduled at %lld (look-ahead %d)", pRecord->triggerID, pRecord->segmentID, in_iSyncTime, in_iLookAheadDuration );
	return AK_Success;
}

/** 2012 0x9632c0, 2013 0x95fb00: a child context of the stinger's segment, prepared at its entry cue with its pre-entry. */
CAkMatrixAwareCtx * CAkMatrixSequencer::CreateStingerCtx( AkUniqueID in_segmentID, AkInt32 & out_iLookAheadDuration )
{
	CAkMusicNode * pNode = static_cast<CAkMusicNode *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_segmentID ) );
	if( !pNode )
	{
		out_iLookAheadDuration = 0;
		return NULL;
	}
	CAkMatrixAwareCtx * pCtx = pNode->CreateContext( m_pOwner, m_pGameObj, m_UserParams );
	if( pCtx )
	{
		AkMusicTransDestRule ruleEntry;
		ruleEntry.eEntryType = EntryTypeEntryMarker;
		ruleEntry.bPlayPreEntry = 1;
		ruleEntry.bDestMatchSourceCueName = 0;
		ruleEntry.fadeParams.iFadeOffset = 0;
		ruleEntry.fadeParams.transitionTime = 0;
		ruleEntry.uCueFilterHash = 0;
		ruleEntry.uJumpToID = 0;
		AkUInt32 uSelectedCue;
		out_iLookAheadDuration = pCtx->Prepare( &ruleEntry, 0, uSelectedCue, 0 );
	}
	pNode->Release();
	return pCtx;
}

/**
 * 2012 0x963360: stinger contexts start their look-ahead before the sync time, stop after their segment, and process
 * the frame with a cutoff just after their sync time; finished records are dropped after their don't-repeat time.
 */
void CAkMatrixSequencer::ProcessStingers( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration )
{
	const AkInt64 iCurrentFrameEnd = in_iCurrentTime + in_uFrameDuration;
	AkStingerRecord * pPrev = NULL;
	AkStingerRecord * pRecord = m_listPendingStingers.m_pFirst;
	while( pRecord )
	{
		if( pRecord->pStingerCtx )
		{
			const AkInt64 iPlayTime = pRecord->iSyncTime - pRecord->iLookAheadDuration;
			if( iCurrentFrameEnd > iPlayTime )
			{
				if( in_iCurrentTime <= iPlayTime )
				{
					AkMusicFade fadeIn;
					fadeIn.transitionTime = 0;
					fadeIn.eFadeCurve = AkCurveInterpolation_Linear;
					fadeIn.iFadeOffset = 0;
					pRecord->pStingerCtx->_Play( fadeIn );
				}
				const AkInt64 iStopTime = pRecord->iSyncTime + pRecord->iRelativeStopTime;
				if( in_iCurrentTime <= iStopTime && iCurrentFrameEnd > iStopTime )
				{
					TransParams fadeOut;
					fadeOut.TransitionTime = 0;
					fadeOut.eFadeCurve = AkCurveInterpolation_Linear;
					pRecord->pStingerCtx->_Stop( fadeOut, (AkUInt32)( iStopTime - in_iCurrentTime ) );
				}
				CAkMatrixAwareCtx * pCtx = pRecord->pStingerCtx;
				if( ( pCtx->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !pCtx->m_bIsPaused )
				{
					AkCutoffInfo cutoffInfo;
					cutoffInfo.iCutoffTime = pRecord->iSyncTime + 1;
					cutoffInfo.bCutoff = true;
					pCtx->Process( in_iCurrentTime, in_uFrameDuration, cutoffInfo );
					if( pRecord->pStingerCtx->m_eStatus > CtxStatusPlaying )
					{
						CAkMatrixAwareCtx * pDone = pRecord->pStingerCtx;
						pRecord->pStingerCtx = NULL;
						pDone->Release();
					}
				}
			}
		}
		else if( in_iCurrentTime > pRecord->iSyncTime + pRecord->uDontRepeatTime )
		{
			AkStingerRecord * pNext = pRecord->pNextLightItem;
			if( pRecord == m_listPendingStingers.m_pFirst )
			{
				m_listPendingStingers.m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextLightItem = pNext;
			}
			FreeStingerRecord( pRecord );
			pRecord = pNext;
			continue;
		}
		pPrev = pRecord;
		pRecord = pRecord->pNextLightItem;
	}
}

/** 2012 0x963500, 2013 0x95fd40. */
void CAkMatrixSequencer::RemoveAllPendingStingers()
{
	while( AkStingerRecord * pRecord = m_listPendingStingers.m_pFirst )
	{
		m_listPendingStingers.m_pFirst = pRecord->pNextLightItem;
		FreeStingerRecord( pRecord );
	}
}

/**
 * 2012 0x962fb0: the earliest sync point, in the window, of the state sync types the window's segment (tracks, bus)
 * declares for the group. Immediate when the group's sync type is "immediate"; not handled when nothing syncs to it.
 */
CAkMatrixSequencer::AkDelayedStateHandlingResult CAkMatrixSequencer::GetEarliestStateSyncTime( const CAkScheduleWindow & in_window, AkStateGroupID in_stateGroupID, AkInt64 & out_iSyncTime )
{
	out_iSyncTime = 0;
	CAkParameterNodeBase::CAkStateSyncArray stateSyncs;
	CAkMusicSegment * pNode = in_window.GetNode( NULL );
	pNode->GetStateSyncTypes( in_stateGroupID, &stateSyncs );
	AkDelayedStateHandlingResult eResult;
	if( !stateSyncs.m_StateSyncArray.Length() )
	{
		eResult = AK_DelayedStateNotHandled;
	}
	else if( !stateSyncs.m_StateSyncArray[ 0 ] )
	{
		eResult = AK_DelayedStateImmediate;
	}
	else
	{
		eResult = AK_DelayedStateCannotSync;
		for( AkUInt32 * it = stateSyncs.m_StateSyncArray.m_pItems; it != stateSyncs.m_StateSyncArray.m_pItemsEnd; ++it )
		{
			AkUInt32 uCueFilter = 0;
			AkInt64 iSyncTime;
			if( in_window.FindSyncPoint( (AkInt64)m_uTime - m_pOwner->m_iLocalTime, (AkSyncType)*it, uCueFilter, false, false, iSyncTime ) == AK_Success )
			{
				if( eResult == AK_DelayedStateCannotSync || iSyncTime < out_iSyncTime )
				{
					out_iSyncTime = iSyncTime;
					eResult = AK_DelayedStateSyncFound;
				}
			}
		}
	}
	stateSyncs.m_StateSyncArray.Term();
	return eResult;
}

/**
 * 2012 0x963570: how long a state change on the group must wait on this context: the earliest sync point in the
 * playing segment, else in the next one (at its start if it cannot sync), else none (0).
 */
AkInt64 CAkMatrixSequencer::QueryStateChangeDelay( AkStateGroupID in_stateGroupID, AkUInt32 & out_uSegmentLookAhead, AkInt64 & out_iSyncTime )
{
	const AkInt64 iCurrentTime = (AkInt64)m_uTime - m_pOwner->m_iLocalTime;
	CAkScheduleWindow window( m_pOwner, false );
	if( !window.m_itScheduledItem.pItem )
	{
		return 0;
	}
	CAkSegmentCtx * pSegment = window.m_itScheduledItem.pItem->m_pSegment;
	AkDelayedStateHandlingResult eFirstSegmentResult = AK_DelayedStateNotHandled;
	out_uSegmentLookAhead = 0;
	if( pSegment && ( pSegment->m_eStatus & CtxStatusPlaying ) )
	{
		eFirstSegmentResult = GetEarliestStateSyncTime( window, in_stateGroupID, out_iSyncTime );
		if( eFirstSegmentResult == AK_DelayedStateImmediate )
		{
			return 0;
		}
		if( eFirstSegmentResult == AK_DelayedStateSyncFound )
		{
			return out_iSyncTime - iCurrentTime;
		}
	}
	if( !window.m_bIsDurationInfinite )
	{
		m_pOwner->GetNextScheduleWindow( window, false );
		CAkSegmentCtx * pNextSegment = window.m_itScheduledItem.pItem->m_pSegment;
		if( pNextSegment )
		{
			if( eFirstSegmentResult != AK_DelayedStateNotHandled || ( pNextSegment->m_eStatus & CtxStatusPlaying ) )
			{
				out_uSegmentLookAhead = 1;
				switch( GetEarliestStateSyncTime( window, in_stateGroupID, out_iSyncTime ) )
				{
				case AK_DelayedStateSyncFound:
					return out_iSyncTime - iCurrentTime;
				case AK_DelayedStateImmediate:
				case AK_DelayedStateNotHandled:
					if( eFirstSegmentResult != AK_DelayedStateNotHandled )
					{
						out_iSyncTime = window.StartTime();
						return out_iSyncTime - iCurrentTime;
					}
					break;
				default:
					break;
				}
			}
		}
		else if( eFirstSegmentResult == AK_DelayedStateCannotSync && pSegment && ( pSegment->m_eStatus & CtxStatusPlaying ) )
		{
			out_iSyncTime = window.StartTime();
			return out_iSyncTime - iCurrentTime;
		}
	}
	return 0;
}

/** 2012 0x963750, 2013 0x95ff90. */
void CAkMatrixSequencer::OnStopped()
{
	RemoveAllPendingStingers();
}

/** 2012 0x963760, 2013 0x95ffa0. */
void CAkMatrixSequencer::Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples )
{
	ProcessStingers( in_iCurrentTime, in_uNumSamples );
	ProcessMusicNotifications( in_iCurrentTime, in_uNumSamples );
}

/**
 * 2012 0x963790: the trigger's stinger (the deepest node's, merged up the tree) is scheduled at its sync point in the
 * current window, or the next one if the stinger allows a segment look-ahead; one that cannot repeat yet is dropped.
 */
void CAkMatrixSequencer::HandleTrigger( AkTriggerID in_triggerID, bool in_bReschedule )
{
	const AkInt64 iCurrentTime = (AkInt64)m_uTime - m_pOwner->m_iLocalTime;
	CAkScheduleWindow window( m_pOwner, false );
	if( !window.m_itScheduledItem.pItem )
	{
		return;
	}
	AkUInt32 uLookAheadIdx = 0;
	if( in_bReschedule )
	{
		m_pOwner->GetNextScheduleWindow( window, false );
		uLookAheadIdx = 1;
	}
	AKRESULT eResult = AK_Fail;
	const CAkStinger * pStingerData;
	do
	{
		pStingerData = NULL;
		CAkMusicNode * pParentNode = NULL;
		CAkMusicNode * pNode = window.GetNode( &pParentNode );
		if( !pNode )
		{
			pNode = pParentNode;
		}
		CAkMusicNode::CAkStingers stingers;
		pNode->GetStingers( &stingers );
		for( CAkStinger * it = stingers.m_StingerArray.m_pItems; it != stingers.m_StingerArray.m_pItemsEnd; ++it )
		{
			if( it->m_TriggerID == in_triggerID )
			{
				pStingerData = it;
				break;
			}
		}
		if( pStingerData )
		{
			if( !CanPlayStinger( in_triggerID ) )
			{
				stingers.m_StingerArray.Term();
				return;
			}
			AkInt32 iLookAheadDuration;
			CAkMatrixAwareCtx * pStingerCtx = CreateStingerCtx( pStingerData->m_SegmentID, iLookAheadDuration );
			if( !pStingerCtx && pStingerData->m_SegmentID )
			{
				stingers.m_StingerArray.Term();
				return;
			}
			AkUInt32 uCueFilter = pStingerData->m_uCueFilterHash;
			AkInt64 iSyncTime;
			eResult = window.FindSyncPoint( iCurrentTime + iLookAheadDuration, pStingerData->m_SyncPlayAt, uCueFilter, false, false, iSyncTime );
			if( eResult == AK_Success )
			{
				if( ScheduleStingerForPlayback( window, pStingerCtx, pStingerData, iSyncTime, iLookAheadDuration, uLookAheadIdx == 0 ) != AK_Success )
				{
					stingers.m_StingerArray.Term();
					return;
				}
			}
			else
			{
				if( pStingerCtx )
				{
					pStingerCtx->_Cancel();
				}
				if( !window.m_bIsDurationInfinite && pStingerData->m_numSegmentLookAhead == 1 && !uLookAheadIdx )
				{
					m_pOwner->GetNextScheduleWindow( window, false );
					uLookAheadIdx = 1;
				}
				else
				{
					eResult = AK_Success;
				}
			}
		}
		stingers.m_StingerArray.Term();
	}
	while( eResult != AK_Success && pStingerData );
}

/** 2012 0x963a30, 2013 0x960270: cancelled stingers are cancelled (and rescheduled in the next segment when allowed); states re-posted. */
void CAkMatrixSequencer::RescheduleCancelledActions( AkListBareLight<AkAssociatedAction> & in_listActions )
{
	while( AkAssociatedAction * pAction = in_listActions.m_pFirst )
	{
		if( pAction->eActionType == AssocActionTypeState )
		{
			CAkMusicRenderer::m_pMusicRenderer->RescheduleDelayedStateChange( pAction->pStateChangeCookie );
		}
		else if( pAction->eActionType == AssocActionTypeStinger )
		{
			AkStingerRecord * pStingerRecord = pAction->pStingerRecord;
			CAkMatrixAwareCtx * pStingerCtx = pStingerRecord->pStingerCtx;
			const AkTriggerID triggerID = pStingerRecord->triggerID;
			const bool bDoReschedule = pStingerRecord->bCanBeRescheduled;
			if( pStingerCtx )
			{
				pStingerCtx->CancelPlayback( (AkInt64)m_uTime - m_pOwner->m_iLocalTime );
			}
			for( AkStingerRecord * pRecord = m_listPendingStingers.m_pFirst; pRecord; pRecord = pRecord->pNextLightItem )
			{
				if( pRecord == pStingerRecord )
				{
					pRecord->triggerID = 0;
					break;
				}
			}
			if( bDoReschedule )
			{
				HandleTrigger( triggerID, true );
			}
		}
		in_listActions.m_pFirst = pAction->pNextLightItem;
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
	}
}

/** 2012 0x963b20, 2013 0x960360. */
void CAkMatrixSequencer::Trigger( AkTriggerID in_triggerID )
{
	HandleTrigger( in_triggerID, false );
}
