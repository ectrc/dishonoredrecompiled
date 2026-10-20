/*=============================================================================
	AkSegmentCtx.cpp - CAkContextualMusicSequencer + the music actions (akcontextualmusicsequencer.cpp / .h, 2012
	0x96a280..0x96a420, 0x966dc0) and CAkSegmentCtx (aksegmentctx.cpp, 2012 0x966c10..0x967880).

	DISHONORED(port): agent IH4 (wave 26). Times are samples relative to the segment's entry cue; a clip's look-ahead is
	its source's streaming look-ahead (CAkMusicTrack::LookAheadTime) for streamed media that has no usable prefetch.
	Deviations (named):
	  * AkMusicActionPlay::AttachClipAutomation and CAkSegmentCtx::ApplyAutomation are bodies retail inlines (four times in
	    ScheduleAudioClips; in ExecuteScheduledCmds and Process).
	  * ExecuteScheduledCmds sets the play TransParams' curve to linear (retail leaves it uninitialised at time 0).
	  * -akdiag: each clip scheduled and each music PBI started / stopped is logged.
=============================================================================*/
#include "AkSegmentCtx.h"
#include "AkMusicSegment.h"
#include "AkMusicTrack.h"
#include "AkMusicPBI.h"
#include "AkMusicRenderer.h"
#include "AkMatrixSequencer.h"
#include "AkSegmentChain.h"
#include <DishonoredWwiseSilent.h>

// ------------------------------------------------------------------------------------------------------------
// AkMusicActionPlay / CAkContextualMusicSequencer
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x966dc0, 2013 0x963660: the automations not handed to a PBI are freed with the action. */
AkMusicActionPlay::~AkMusicActionPlay()
{
	while( AkMusicAutomation * pAutomation = m_listAutomation.m_pFirst )
	{
		m_listAutomation.m_pFirst = pAutomation->pNextLightItem;
		AK::MemoryMgr::Free( g_DefaultPoolId, pAutomation );
	}
}

/** Inlined in CAkSegmentCtx::ScheduleAudioClips (2012 0x966e30). */
void AkMusicActionPlay::AttachClipAutomation( CAkClipAutomation * in_pAutomation, AkInt32 in_iTimeStart )
{
	AkMusicAutomation * pAutomation = (AkMusicAutomation *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicAutomation ) );
	if( !pAutomation )
	{
		return;
	}
	pAutomation->pAutomationData = in_pAutomation;
	pAutomation->pPBI = NULL;
	pAutomation->iTimeStart = in_iTimeStart;
	m_listAutomation.AddFirst( pAutomation );
}

/** 2012 0x96a280, 2013 0x966aa0. */
CAkContextualMusicSequencer::CAkContextualMusicSequencer()
{
}

/** 2012 0x96a2a0, 2013 0x966ac0. */
CAkContextualMusicSequencer::~CAkContextualMusicSequencer()
{
	RemoveAll();
}

/** 2012 0x96a2c0, 2013 0x966ae0: the first action due before the end of the frame. */
AKRESULT CAkContextualMusicSequencer::PopImminentAction( AkInt32 in_iNow, AkInt32 in_iFrameDuration, AkMusicAction *& out_pAction )
{
	if( m_pFirst && m_pFirst->m_iTime < in_iFrameDuration + in_iNow )
	{
		out_pAction = RemoveFirst();
		return AK_DataReady;
	}
	out_pAction = NULL;
	return AK_NoMoreData;
}

/** 2012 0x96a320, 2013 0x966b40. */
void CAkContextualMusicSequencer::Flush()
{
	while( AkMusicAction * pAction = RemoveFirst() )
	{
		pAction->~AkMusicAction();
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
	}
}

/** 2012 0x96a3b0, 2013 0x966bd0: inserted after every action at or before its time. */
void CAkContextualMusicSequencer::ScheduleAction( AkMusicAction * in_pAction )
{
	AkMusicAction * pPrev = NULL;
	for( AkMusicAction * pAction = m_pFirst; pAction; pAction = pAction->pNextItem )
	{
		if( in_pAction->m_iTime < pAction->m_iTime )
		{
			in_pAction->pNextItem = pAction;
			if( pPrev )
			{
				pPrev->pNextItem = in_pAction;
			}
			else
			{
				m_pFirst = in_pAction;
			}
			++m_ulNumListItems;
			return;
		}
		pPrev = pAction;
	}
	AddLast( in_pAction );
}

/** 2012 0x96a420, 2013 0x966c40: the stop actions aimed at a PBI that goes away. */
void CAkContextualMusicSequencer::ClearActionsByTarget( CAkMusicPBI * in_pTarget )
{
	AkMusicAction * pPrev = NULL;
	AkMusicAction * pAction = m_pFirst;
	while( pAction )
	{
		if( pAction->Type() == MusicActionTypeStop && static_cast<AkMusicActionStop *>( pAction )->pTargetPBI == in_pTarget )
		{
			AkMusicAction * pNext = pAction->pNextItem;
			if( pAction == m_pFirst )
			{
				m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextItem = pNext;
			}
			if( pAction == m_pLast )
			{
				m_pLast = pPrev;
			}
			--m_ulNumListItems;
			pAction->~AkMusicAction();
			AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
			pAction = pNext;
		}
		else
		{
			pPrev = pAction;
			pAction = pAction->pNextItem;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkSegmentCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9671c0, 2013 0x963a60: the segment and its tracks are referenced (under the node index lock). */
CAkSegmentCtx::CAkSegmentCtx( CAkMusicSegment * in_pSegmentNode, CAkMusicCtx * in_pParentCtx )
	: CAkMusicCtx( in_pParentCtx )
	, m_pSegmentNode( in_pSegmentNode )
	, m_pOwner( NULL )
	, m_iAudibleTime( 0 )
{
	m_listAutomation.m_pFirst = NULL;
	if( !m_pSegmentNode )
	{
		return;
	}
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
	m_pSegmentNode->AddRef();
	const AkUInt16 uNumTracks = m_pSegmentNode->Children();
	if( uNumTracks && m_arTracks.Reserve( uNumTracks ) != AK_Success )
	{
		return;
	}
	for( AkUInt16 iTrack = 0; iTrack < uNumTracks; ++iTrack )
	{
		CAkMusicTrack * pTrack = m_pSegmentNode->Track( iTrack );
		pTrack->AddRef();
		m_arTracks.AddLast( pTrack );
	}
}

/** 2012 0x967320, 2013 0x963bc0. */
CAkSegmentCtx::~CAkSegmentCtx()
{
	m_arTrackRS.Term();
	if( m_pSegmentNode )
	{
		AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
		for( CAkMusicTrack ** it = m_arTracks.m_pItems; it != m_arTracks.m_pItemsEnd; ++it )
		{
			( *it )->Release();
		}
		m_pSegmentNode->Release();
	}
	m_arTracks.Term();
}

/** 2012 0x9673e0, 2013 0x963c80: each track picks its sub-track for this playback. */
AKRESULT CAkSegmentCtx::Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	CAkMusicCtx::Init( in_GameObject, in_rUserparams );
	const AkUInt32 uNumTracks = m_arTracks.Length();
	if( uNumTracks && m_arTrackRS.Reserve( uNumTracks ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	for( CAkMusicTrack ** it = m_arTracks.m_pItems; it != m_arTracks.m_pItemsEnd; ++it )
	{
		m_arTrackRS.AddLast( ( *it )->GetNextRS() );
	}
	return AK_Success;
}

/** 2012 0x966c10, 2013 0x963410. */
void CAkSegmentCtx::OnPaused()
{
	CAkMusicCtx::OnPaused();
	if( m_pOwner )
	{
		m_pOwner->OnPaused();
	}
}

/**
 * 2012 0x966c30: where the clip of a music PBI coming back from virtual would play now (source offset), and how long
 * its stream needs before it (frame offset, rounded up to whole audio frames); false if the clip is over.
 */
bool CAkSegmentCtx::GetSourceInfoForPlaybackRestart( const CAkMusicPBI * in_pCtx, AkInt32 & out_iLookAhead, AkInt32 & out_iSourceOffset )
{
	if( m_eStatus > CtxStatusPlaying )
	{
		return false;
	}
	CAkSource * pSource = in_pCtx->m_pSource;
	AkInt32 iLookAhead = pSource->m_sSrcTypeInfo.mediaInfo.Type == 1 ? (AkInt32)static_cast<CAkMusicSource *>( pSource )->m_uStreamingLookAhead : 0;
	iLookAhead = (AkInt32)( ( (AkUInt32)iLookAhead + AK_NUM_VOICE_REFILL_FRAMES - 1 ) / AK_NUM_VOICE_REFILL_FRAMES * AK_NUM_VOICE_REFILL_FRAMES );
	out_iLookAhead = iLookAhead;
	CAkChainCtx * pParent = static_cast<CAkChainCtx *>( m_pParentCtx );
	const AkTrackSrc * pSrcInfo = in_pCtx->m_pSrcInfo;
	const AkUInt32 uNumFramesElapsed = pParent->m_pSequencer->m_uCurTimeWindowSize;
	const AkInt32 iSegmentPosition = pParent->GetSegmentPosition( m_pOwner );
	const AkInt32 iPosition = iSegmentPosition - (AkInt32)uNumFramesElapsed + m_pSegmentNode->PreEntryDuration();
	if( iLookAhead + iPosition >= (AkInt32)( pSrcInfo->uClipStartPosition + pSrcInfo->uClipDuration ) )
	{
		return false;
	}
	const AkInt32 iSourceOffset = (AkInt32)( (AkUInt32)( iLookAhead + pSrcInfo->iSourceTrimOffset - (AkInt32)pSrcInfo->uClipStartPosition + iPosition ) % pSrcInfo->uSrcDuration );
	out_iSourceOffset = iSourceOffset < 0 ? 0 : iSourceOffset;
	return true;
}

/** 2012 0x966cf0, 2013 0x963590: how early the context must start for every clip's stream to be ready at in_iPosition. */
AkInt32 CAkSegmentCtx::ComputeMinSrcLookAhead( AkInt32 in_iPosition )
{
	const AkInt32 iPosition = m_pSegmentNode->PreEntryDuration() + in_iPosition;
	AkInt32 iContextLookAhead = 0;
	AkUInt32 uTrack = 0;
	for( CAkMusicTrack ** it = m_arTracks.m_pItems; it != m_arTracks.m_pItemsEnd; ++it, ++uTrack )
	{
		CAkMusicTrack * pTrack = *it;
		for( AkTrackSrc * pClip = pTrack->m_arTrackPlaylist.m_pItems; pClip != pTrack->m_arTrackPlaylist.m_pItemsEnd; ++pClip )
		{
			if( pClip->uSubTrackIndex != m_arTrackRS[ uTrack ] )
			{
				continue;
			}
			CAkMusicSource * pSource = pTrack->GetSourcePtr( pClip->srcID );
			if( !pSource )
			{
				continue;
			}
			AkInt32 iClipStartRelative = (AkInt32)pClip->uClipStartPosition - iPosition;
			AkInt32 iSrcLookAhead = 0;
			if( pSource->m_sSrcTypeInfo.mediaInfo.Type != 1 )
			{
				if( iClipStartRelative < 0 )
				{
					iClipStartRelative = 0;
				}
			}
			else if( !pSource->m_sSrcTypeInfo.mediaInfo.bPrefetch || iClipStartRelative < 0 )
			{
				iSrcLookAhead = (AkInt32)pSource->m_uStreamingLookAhead;
				if( iClipStartRelative < 0 )
				{
					iClipStartRelative = 0;
				}
			}
			const AkInt32 iLookAhead = iSrcLookAhead - iClipStartRelative;
			if( iContextLookAhead < iLookAhead )
			{
				iContextLookAhead = iLookAhead;
			}
		}
	}
	return iContextLookAhead;
}

/**
 * 2012 0x966e30: one play action per clip of the chosen sub-tracks still audible from the context's audible start,
 * early by the clip's look-ahead; a clip already started plays from the matching source offset. Each clip's automation
 * curves (volume, LPF, fade-in, fade-out) ride on its action.
 */
void CAkSegmentCtx::ScheduleAudioClips()
{
	const AkInt32 iEffectiveSegmentPosition = m_iAudibleTime + m_pSegmentNode->PreEntryDuration();
	AkUInt32 uTrack = 0;
	for( CAkMusicTrack ** it = m_arTracks.m_pItems; it != m_arTracks.m_pItemsEnd; ++it, ++uTrack )
	{
		CAkMusicTrack * pTrack = *it;
		const AkUInt32 uNumClips = pTrack->m_arTrackPlaylist.Length();
		for( AkUInt32 uClip = 0; uClip < uNumClips; ++uClip )
		{
			const AkTrackSrc & rClip = pTrack->m_arTrackPlaylist[ uClip ];
			if( rClip.uSubTrackIndex != m_arTrackRS[ uTrack ] )
			{
				continue;
			}
			CAkMusicSource * pSource = pTrack->GetSourcePtr( rClip.srcID );
			if( !pSource )
			{
				continue;
			}
			AkUInt32 uLookAhead = 0;
			if( pSource->m_sSrcTypeInfo.mediaInfo.Type == 1 && ( !pSource->m_sSrcTypeInfo.mediaInfo.bPrefetch || ( iEffectiveSegmentPosition > (AkInt32)rClip.uClipStartPosition && !rClip.iSourceTrimOffset ) ) )
			{
				uLookAhead = pSource->m_uStreamingLookAhead;
			}
			if( iEffectiveSegmentPosition >= (AkInt32)( rClip.uClipStartPosition + rClip.uClipDuration ) )
			{
				continue;
			}
			AkInt32 iPlayTime;
			AkInt32 iSourceOffset;
			if( iEffectiveSegmentPosition <= (AkInt32)rClip.uClipStartPosition )
			{
				iPlayTime = (AkInt32)rClip.uClipStartPosition - (AkInt32)uLookAhead;
				iSourceOffset = rClip.iSourceTrimOffset;
			}
			else
			{
				iPlayTime = iEffectiveSegmentPosition - (AkInt32)uLookAhead;
				iSourceOffset = (AkInt32)( (AkUInt32)( iEffectiveSegmentPosition + rClip.iSourceTrimOffset - (AkInt32)rClip.uClipStartPosition ) % rClip.uSrcDuration );
			}
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicActionPlay ) );
			if( !pMem )
			{
				continue;
			}
			AkMusicActionPlay * pAction = AkPlacementNew( pMem ) AkMusicActionPlay( iPlayTime - m_pSegmentNode->PreEntryDuration(), pTrack, rClip, (AkUInt32)iSourceOffset, uLookAhead );
			for( AkUInt32 eType = AutomationType_Volume; eType <= AutomationType_FadeOut; ++eType )
			{
				CAkClipAutomation * pAutomation = pTrack->GetClipAutomation( uClip, (AkClipAutomationType)eType );
				if( pAutomation )
				{
					pAction->AttachClipAutomation( pAutomation, (AkInt32)rClip.uClipStartPosition );
				}
			}
			m_sequencer.ScheduleAction( pAction );
			AKDIAG( L"akdiag: music clip scheduled: segment 0x%08x track 0x%08x sub-track %u source 0x%08x at %d (source offset %d, look-ahead %u, clip %u..%u samples)",
				m_pSegmentNode->key, pTrack->key, rClip.uSubTrackIndex, rClip.srcID, pAction->m_iTime, iSourceOffset, uLookAhead, rClip.uClipStartPosition, rClip.uClipStartPosition + rClip.uClipDuration );
		}
	}
}

/** 2012 0x967150, 2013 0x9639f0: the context becomes audible at in_iSourceOffset (segment position); returns the look-ahead needed. */
AkInt32 CAkSegmentCtx::Prepare( AkInt32 in_iSourceOffset )
{
	m_iAudibleTime = in_iSourceOffset;
	return ComputeMinSrcLookAhead( in_iSourceOffset );
}

/** 2012 0x967160, 2013 0x963a00. */
void CAkSegmentCtx::OnPlayed()
{
	CAkMusicCtx::OnPlayed();
	ScheduleAudioClips();
}

/** 2012 0x967170, 2013 0x963a10. */
void CAkSegmentCtx::Flush()
{
	while( AkMusicAutomation * pAutomation = m_listAutomation.m_pFirst )
	{
		m_listAutomation.m_pFirst = pAutomation->pNextLightItem;
		AK::MemoryMgr::Free( g_DefaultPoolId, pAutomation );
	}
	m_sequencer.Flush();
}

/** 2012 0x967490, 2013 0x963d30: the scheduled item lets go of this context. */
void CAkSegmentCtx::OnStopped()
{
	AddRef();
	Flush();
	if( m_pOwner )
	{
		CAkScheduledItem * pOwner = m_pOwner;
		pOwner->m_pSegment->m_pOwner = NULL;
		CAkSegmentCtx * pSegment = pOwner->m_pSegment;
		pOwner->m_pSegment = NULL;
		if( pSegment )
		{
			pSegment->Release();
		}
	}
	CAkMusicCtx::OnStopped();
	Release();
}

/** Inlined in retail (ExecuteScheduledCmds 0x9674d0, Process 0x967880): the curve's value at the end of the frame. */
void CAkSegmentCtx::ApplyAutomation( AkMusicAutomation * in_pAutomation, AkInt32 in_iSegmentFrameEnd )
{
	AkUInt32 uIndex = 0;
	const AkReal32 fValue = in_pAutomation->pAutomationData->m_tableAutomation.ConvertInternal( (AkReal32)( in_iSegmentFrameEnd - in_pAutomation->iTimeStart ), uIndex );
	in_pAutomation->pPBI->SetAutomationValue( in_pAutomation->pAutomationData->m_eAutoType, fValue );
}

/**
 * 2012 0x9674d0: the actions due in this frame: a play creates the clip's music PBI through the music renderer (at its
 * source offset, delayed into the frame by the action's offset plus its look-ahead) and schedules its stop at the clip
 * end; a stop truncates its PBI at the frame offset and applies its automation one last time.
 */
void CAkSegmentCtx::ExecuteScheduledCmds( AkInt32 in_iTime, AkUInt32 in_uNumSamples )
{
	AkMusicAction * pAction;
	while( m_sequencer.PopImminentAction( in_iTime, (AkInt32)in_uNumSamples, pAction ) == AK_DataReady )
	{
		const AkUInt32 uAudioFrameOffset = (AkUInt32)( pAction->m_iTime - in_iTime );
		if( pAction->Type() == MusicActionTypePlay )
		{
			AkMusicActionPlay * pActionPlay = static_cast<AkMusicActionPlay *>( pAction );
			TransParams transParams;
			transParams.TransitionTime = 0;
			transParams.eFadeCurve = AkCurveInterpolation_Linear;
			CAkMusicPBI * pSrcCtx = NULL;
			CAkMatrixSequencer * pSequencer = static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->m_pSequencer;
			const AkTrackSrc * pSrcInfo = pActionPlay->m_rTrackSrc;
			CAkMusicSource * pSource = pActionPlay->m_pTrack->GetSourcePtr( pSrcInfo->srcID );
			if( CAkMusicRenderer::Play( this, pActionPlay->m_pTrack, pSource, pSequencer->m_pGameObj, transParams, pSequencer->m_UserParams, pSrcInfo, pActionPlay->m_uSourceOffset, pActionPlay->m_uPlayOffset + uAudioFrameOffset, pSrcCtx ) == AK_Success )
			{
				AKDIAG( L"akdiag: music PBI started: segment 0x%08x track 0x%08x source 0x%08x playing id %u (source offset %u, frame offset %u)",
					m_pSegmentNode->key, pActionPlay->m_pTrack->key, pSrcInfo->srcID, pSequencer->m_UserParams.m_PlayingID, pActionPlay->m_uSourceOffset, pActionPlay->m_uPlayOffset + uAudioFrameOffset );
				if( m_eStatus & CtxStatusLastFrame )
				{
					pSrcCtx->_Stop( m_uNumLastSamples );
					while( AkMusicAutomation * pAutomation = pActionPlay->m_listAutomation.m_pFirst )
					{
						pActionPlay->m_listAutomation.m_pFirst = pAutomation->pNextLightItem;
						pAutomation->pPBI = pSrcCtx;
						ApplyAutomation( pAutomation, (AkInt32)in_uNumSamples + in_iTime + m_pSegmentNode->PreEntryDuration() );
						AK::MemoryMgr::Free( g_DefaultPoolId, pAutomation );
					}
				}
				else
				{
					void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicActionStop ) );
					if( pMem )
					{
						AkMusicActionStop * pStop = AkPlacementNew( pMem ) AkMusicActionStop( (AkInt32)( pSrcInfo->uClipStartPosition + pSrcInfo->uClipDuration ) - m_pSegmentNode->PreEntryDuration(), pSrcCtx );
						m_sequencer.ScheduleAction( pStop );
						while( AkMusicAutomation * pAutomation = pActionPlay->m_listAutomation.m_pFirst )
						{
							pActionPlay->m_listAutomation.m_pFirst = pAutomation->pNextLightItem;
							pAutomation->pPBI = pSrcCtx;
							pStop->bHasAutomation = true;
							m_listAutomation.AddFirst( pAutomation );
							if( pAutomation->pAutomationData->m_eAutoType == AutomationType_FadeIn )
							{
								pSrcCtx->FixStartTimeForFadeIn();
							}
							else if( pAutomation->pAutomationData->m_eAutoType == AutomationType_FadeOut )
							{
								pSrcCtx->m_bNeedsFadeOut = 1;
							}
						}
					}
					else
					{
						pSrcCtx->_Stop( (AkUInt32)0 );
					}
				}
			}
		}
		else if( pAction->Type() == MusicActionTypeStop )
		{
			AkMusicActionStop * pStop = static_cast<AkMusicActionStop *>( pAction );
			CAkMusicPBI * pTarget = pStop->pTargetPBI;
			pTarget->_Stop( uAudioFrameOffset );
			AKDIAG( L"akdiag: music PBI stopped: segment 0x%08x at frame offset %u", m_pSegmentNode->key, uAudioFrameOffset );
			if( pStop->bHasAutomation )
			{
				AkMusicAutomation * pPrev = NULL;
				AkMusicAutomation * pAutomation = m_listAutomation.m_pFirst;
				while( pAutomation )
				{
					if( pAutomation->pPBI == pTarget )
					{
						ApplyAutomation( pAutomation, (AkInt32)in_uNumSamples + in_iTime + m_pSegmentNode->PreEntryDuration() );
						AkMusicAutomation * pNext = pAutomation->pNextLightItem;
						if( pAutomation == m_listAutomation.m_pFirst )
						{
							m_listAutomation.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						AK::MemoryMgr::Free( g_DefaultPoolId, pAutomation );
						pAutomation = pNext;
					}
					else
					{
						pPrev = pAutomation;
						pAutomation = pAutomation->pNextLightItem;
					}
				}
			}
		}
		pAction->~AkMusicAction();
		AK::MemoryMgr::Free( g_DefaultPoolId, pAction );
	}
}

/** 2012 0x967800, 2013 0x9640a0: a PBI going away takes its stop actions and automations with it. */
void CAkSegmentCtx::RemoveAllReferences( CAkPBI * in_pCtx )
{
	CAkMusicPBI * pMusicPBI = static_cast<CAkMusicPBI *>( in_pCtx );
	m_sequencer.ClearActionsByTarget( pMusicPBI );
	AkMusicAutomation * pPrev = NULL;
	AkMusicAutomation * pAutomation = m_listAutomation.m_pFirst;
	while( pAutomation )
	{
		if( pAutomation->pPBI == pMusicPBI )
		{
			AkMusicAutomation * pNext = pAutomation->pNextLightItem;
			if( pAutomation == m_listAutomation.m_pFirst )
			{
				m_listAutomation.m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextLightItem = pNext;
			}
			AK::MemoryMgr::Free( g_DefaultPoolId, pAutomation );
			pAutomation = pNext;
		}
		else
		{
			pPrev = pAutomation;
			pAutomation = pAutomation->pNextLightItem;
		}
	}
}

/** 2012 0x967880, 2013 0x964120: the frame's commands (until the stop sample when stopping), then the automation curves. */
void CAkSegmentCtx::Process( AkInt32 in_iTime, AkUInt32 in_uNumSamples )
{
	AddRef();
	AkUInt32 uNumSamples = in_uNumSamples;
	if( ( m_eStatus & CtxStatusLastFrame ) && m_uNumLastSamples != (AkUInt32)-1 )
	{
		uNumSamples = m_uNumLastSamples;
	}
	ExecuteScheduledCmds( in_iTime, uNumSamples );
	const AkInt32 iSegmentFrameEnd = (AkInt32)uNumSamples + in_iTime + m_pSegmentNode->PreEntryDuration();
	for( AkMusicAutomation * pAutomation = m_listAutomation.m_pFirst; pAutomation; pAutomation = pAutomation->pNextLightItem )
	{
		ApplyAutomation( pAutomation, iSegmentFrameEnd );
	}
	if( m_eStatus & CtxStatusLastFrame )
	{
		OnStopped();
	}
	Release();
}
