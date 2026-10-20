/*=============================================================================
	AkSequenceCtx.cpp - CAkSequenceCtx (aksequencectx.cpp, 2012 0x96ca00..0x96d070).

	DISHONORED(port): agent IH4 (wave 26).
	Deviations (named):
	  * The transition rule ScheduleNextSegment builds for a transition segment has its destination jump id set to 0
	    (retail leaves that stack field unset; a playlist never jumps).
	  * -akdiag: every segment appended to a playlist chain is logged with the rule's sync.
=============================================================================*/
#include "AkSequenceCtx.h"
#include "AkMusicSegment.h"
#include "AkSegmentCtx.h"
#include "AkMatrixSequencer.h"
#include <DishonoredWwiseSilent.h>

/** 2012 0x96ca00, 2013 0x9691d0. */
CAkSequenceCtx::CAkSequenceCtx( CAkMusicRanSeqCntr * in_pSequenceNode, CAkMusicCtx * in_pParentCtx )
	: CAkChainCtx( in_pParentCtx )
	, m_pSequenceNode( in_pSequenceNode )
	, m_PlayListIterator( in_pSequenceNode )
	, m_bIsChainValid( true )
{
	if( m_pSequenceNode )
	{
		m_pSequenceNode->AddRef();
	}
}

/** 2012 0x96ca90 (the deleting destructor's body). */
CAkSequenceCtx::~CAkSequenceCtx()
{
	m_PlayListIterator.Term();
	if( m_pSequenceNode )
	{
		m_pSequenceNode->Release();
	}
}

/** 2012 0x96ca50. */
void CAkSequenceCtx::EndInit()
{
	m_PlayListIterator.EndInit();
}

/** Retail slot 15 (FDefaultMaterialInstance::IsSelected folded): the playlist container. */
CAkMusicNode * CAkSequenceCtx::Node()
{
	return m_pSequenceNode;
}

/** 2012 0x96ca60, 2013 0x969240: the chain stops growing; the last item stands. */
CAkScheduledItem * CAkSequenceCtx::HandleFatalError()
{
	m_PlayListIterator.m_bIsSegmentValid = false;
	m_bIsChainValid = false;
	return m_chain.Last();
}

/** 2012 0x96ca70, 2013 0x969250. */
bool CAkSequenceCtx::IsValid()
{
	return m_bIsChainValid;
}

/**
 * 2012 0x96caf0: the source item (if any) gets its stop at the exit sync point (shifted by the fade and its offset,
 * or past the post-exit); the next segment is enqueued at that sync point, prepared at its entry position (minus the
 * pre-entry when played) with the rule's fade-in.
 */
CAkScheduledItem * CAkSequenceCtx::AppendItem( const AkMusicTransitionRule & in_rule, CAkScheduledItem * in_pSrcItem, AkUniqueID in_nextID, AkUniqueID /*in_playlistItemID*/ )
{
	CAkSegmentCtx * pSegmentCtx = NULL;
	AkUInt64 uCurrentChainTime = 0;
	AkInt32 iSyncOffset = 0;
	if( in_pSrcItem )
	{
		uCurrentChainTime = in_pSrcItem->m_iLocalTime;
		AkUInt32 uCueFilter = in_rule.srcRule.uCueFilterHash;
		CAkMusicSegment * pSrcSegmentNode = in_pSrcItem->m_pSegment->m_pSegmentNode;
		AkUInt32 uSyncPos = 0;
		pSrcSegmentNode->GetExitSyncPos( 0, (AkSyncType)in_rule.srcRule.eSyncType, uCueFilter, true, uSyncPos );
		AkTimeMs transitionTime = in_rule.srcRule.fadeParams.transitionTime;
		iSyncOffset = (AkInt32)uSyncPos;
		AkInt32 iStopOffset = in_rule.srcRule.fadeParams.iFadeOffset - transitionTime * (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
		if( iStopOffset < -(AkInt32)uSyncPos )
		{
			iStopOffset = -(AkInt32)uSyncPos;
			transitionTime = AkTimeConv::SamplesToMilliseconds( in_rule.srcRule.fadeParams.iFadeOffset + (AkInt32)uSyncPos );
		}
		if( !iStopOffset && !transitionTime && in_rule.srcRule.bPlayPostExit )
		{
			iStopOffset = (AkInt32)pSrcSegmentNode->PostExitDuration();
		}
		in_pSrcItem->AttachStopCmd( transitionTime, in_rule.srcRule.fadeParams.eFadeCurve, (AkInt32)pSrcSegmentNode->ActiveDuration() + iStopOffset );
	}
	const AkMusicTransDestRule & destRule = in_rule.destRule;
	AkUInt32 uEntrySyncPos = 0;
	if( in_nextID )
	{
		CAkMusicSegment * pNextSegment = static_cast<CAkMusicSegment *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_nextID ) );
		if( pNextSegment )
		{
			AkUInt32 uSelectedCue;
			pNextSegment->GetEntrySyncPos( destRule, 0, 0, uSelectedCue, uEntrySyncPos );
			pSegmentCtx = pNextSegment->CreateLowLevelSegmentCtxAndAddRef( this, m_pSequencer->m_pGameObj, m_pSequencer->m_UserParams );
			if( !pSegmentCtx )
			{
				m_PlayListIterator.m_bIsSegmentValid = false;
			}
			pNextSegment->Release();
		}
		else
		{
			m_PlayListIterator.m_bIsSegmentValid = false;
		}
	}
	CAkScheduledItem * pItem = EnqueueItem( (AkUInt64)( (AkInt64)iSyncOffset + (AkInt64)uCurrentChainTime ), pSegmentCtx );
	if( pSegmentCtx )
	{
		pSegmentCtx->Release();
	}
	if( !pItem )
	{
		return HandleFatalError();
	}
	if( pSegmentCtx )
	{
		const AkInt32 iStartPosition = destRule.bPlayPreEntry ? (AkInt32)uEntrySyncPos - pSegmentCtx->m_pSegmentNode->PreEntryDuration() : (AkInt32)uEntrySyncPos;
		const AkInt32 iLookAhead = pItem->Prepare( uEntrySyncPos, iStartPosition, destRule.fadeParams );
		pItem->SetFadeIn( destRule.fadeParams.transitionTime, destRule.fadeParams.eFadeCurve, iLookAhead + destRule.fadeParams.iFadeOffset );
		AKDIAG( L"akdiag: music playlist 0x%08x: segment 0x%08x appended at chain time %llu (exit sync type %u at %d, entry %u, fade-in %d ms)",
			m_pSequenceNode->key, in_nextID, pItem->m_iLocalTime, (AkUInt32)in_rule.srcRule.eSyncType, iSyncOffset, uEntrySyncPos, destRule.fadeParams.transitionTime );
	}
	return pItem;
}

/**
 * 2012 0x96ccc0: the playlist's next segment, through the transition rule from the chain's last segment; a rule with
 * a transition segment appends it first, then the destination after the transition segment's exit cue.
 */
CAkScheduledItem * CAkSequenceCtx::ScheduleNextSegment( bool & out_bPlayPreEntry )
{
	out_bPlayPreEntry = false;
	if( m_eStatus > CtxStatusPlaying )
	{
		return NULL;
	}
	const AkUniqueID nextID = m_PlayListIterator.m_actualSegment;
	if( !m_PlayListIterator.m_bIsSegmentValid )
	{
		return NULL;
	}
	m_PlayListIterator.JumpNext();
	CAkScheduledItem * pLast = m_chain.Last();
	const AkUniqueID srcID = pLast ? pLast->m_pSegment->m_pSegmentNode->key : 0;
	const AkMusicTransitionRule & rule = m_pSequenceNode->GetTransitionRule( srcID, nextID );
	if( !rule.pTransObj )
	{
		out_bPlayPreEntry = rule.destRule.bPlayPreEntry != 0;
		return AppendItem( rule, pLast, nextID, 0 );
	}
	AkMusicTransitionRule transRule;
	transRule.srcRule = rule.srcRule;
	transRule.destRule.fadeParams = rule.pTransObj->fadeInParams;
	transRule.destRule.uCueFilterHash = 0;
	transRule.destRule.uJumpToID = 0;
	transRule.destRule.eEntryType = EntryTypeEntryMarker;
	transRule.destRule.bPlayPreEntry = rule.pTransObj->bPlayPreEntry;
	transRule.destRule.bDestMatchSourceCueName = 0;
	out_bPlayPreEntry = rule.pTransObj->bPlayPreEntry != 0;
	CAkScheduledItem * pAppended = AppendItem( transRule, pLast, rule.pTransObj->segmentID, 0 );
	if( pAppended )
	{
		if( !pAppended->m_pSegment )
		{
			return HandleFatalError();
		}
		transRule.srcRule.fadeParams = rule.pTransObj->fadeOutParams;
		transRule.srcRule.eSyncType = SyncTypeExitMarker;
		transRule.srcRule.bPlayPostExit = rule.pTransObj->bPlayPostExit;
		transRule.destRule = rule.destRule;
		AppendItem( transRule, pAppended, nextID, 0 );
	}
	return pAppended;
}

/** 2012 0x96ce90, 2013 0x969670: the iterator's first segment is appended and the context prepared (from its pre-entry if played). */
AKRESULT CAkSequenceCtx::Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	AKRESULT eResult = CAkMatrixAwareCtx::Init( in_GameObject, in_rUserparams );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = m_PlayListIterator.Init();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	bool bPlayPreEntry;
	CAkScheduledItem * pItem = ScheduleNextSegment( bPlayPreEntry );
	if( !pItem || !pItem->m_pSegment )
	{
		return AK_Fail;
	}
	const AkInt32 iStartPosition = bPlayPreEntry ? -pItem->m_pSegment->m_pSegmentNode->PreEntryDuration() : 0;
	AkUInt32 uSelectedCue;
	m_iLocalTime = Prepare( NULL, iStartPosition, uSelectedCue, 0 );
	return AK_Success;
}

/**
 * 2012 0x96cf10: the chain is grown until an item starts at or after the desired position, then until three more
 * items (buckets) lie ahead of it; more than 64 segments in one call is a fatal error (an empty playlist loop).
 */
void CAkSequenceCtx::UpdateChainLength( AkInt64 in_iDesiredPosition )
{
	AkUInt32 uNumSegmentsAdded = 0;
	AkUInt32 uBucketLookAhead = 0;
	CAkScheduledItem * pPrev = m_chain.First();
	CAkScheduledItem * pItem = m_chain.First();
	for( ; pItem; pItem = pItem->pNextItem )
	{
		if( (AkInt64)pItem->m_iLocalTime >= in_iDesiredPosition )
		{
			break;
		}
		pPrev = pItem;
	}
	if( !pItem )
	{
		pItem = pPrev;
		if( !pItem )
		{
			return;
		}
		if( (AkInt64)pItem->m_iLocalTime < in_iDesiredPosition )
		{
			for( ;; )
			{
				if( !m_PlayListIterator.m_bIsSegmentValid )
				{
					return;
				}
				CAkScheduledItem * pLast = m_chain.Last();
				bool bPlayPreEntry;
				CAkScheduledItem * pNew = ScheduleNextSegment( bPlayPreEntry );
				if( pNew == pLast || !pNew )
				{
					return;
				}
				if( ++uNumSegmentsAdded > 64 )
				{
					HandleFatalError();
					return;
				}
				pPrev = pItem;
				pItem = pItem->pNextItem;
				if( (AkInt64)pItem->m_iLocalTime >= in_iDesiredPosition )
				{
					break;
				}
			}
		}
	}
	for( ;; )
	{
		++uBucketLookAhead;
		if( pItem )
		{
			pPrev = pItem;
			pItem = pItem->pNextItem;
		}
		else
		{
			if( !m_PlayListIterator.m_bIsSegmentValid )
			{
				return;
			}
			CAkScheduledItem * pLast = m_chain.Last();
			bool bPlayPreEntry;
			CAkScheduledItem * pNew = ScheduleNextSegment( bPlayPreEntry );
			if( pNew == pLast || !pNew )
			{
				return;
			}
			if( ++uNumSegmentsAdded > 64 )
			{
				HandleFatalError();
				return;
			}
			pPrev = pPrev->pNextItem;
			pItem = pPrev->pNextItem;
		}
		if( uBucketLookAhead >= 3 )
		{
			return;
		}
	}
}

/** 2012 0x96d010, 2013 0x9697f0: a "jump to playlist item" destination: the chain restarts at that item. */
CAkScheduledItem * CAkSequenceCtx::JumpToSegment( AkUniqueID in_uJumpToID )
{
	if( !in_uJumpToID )
	{
		return m_chain.First();
	}
	Flush();
	if( m_PlayListIterator.JumpTo( in_uJumpToID ) != AK_Success )
	{
		return HandleFatalError();
	}
	bool bPlayPreEntry;
	CAkScheduledItem * pItem = ScheduleNextSegment( bPlayPreEntry );
	if( pItem && !pItem->m_pSegment )
	{
		Flush();
		return NULL;
	}
	return pItem;
}

/** 2012 0x96d070, 2013 0x969850. */
bool CAkSequenceCtx::Grow()
{
	if( !m_PlayListIterator.m_bIsSegmentValid )
	{
		return false;
	}
	CAkScheduledItem * pLast = m_chain.Last();
	bool bPlayPreEntry;
	CAkScheduledItem * pNew = ScheduleNextSegment( bPlayPreEntry );
	return pNew && pNew != pLast;
}
