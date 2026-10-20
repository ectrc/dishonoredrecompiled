/*=============================================================================
	AkMusicSwitchCtx.cpp - CAkNothingCtx, CAkMusicSwitchTransition and CAkMusicSwitchCtx (akmusicswitchctx.cpp, 2012
	0x96a4a0..0x96c140).

	DISHONORED(port): agent IH4 (wave 26). ScheduleSwitchTransition (2233 bytes) is rebuilt from both decompiles
	(2012 0x96b480 and its byte-identical 2013 twin 0x967ca0, whose frame recovers better) and the disassembly.
	Deviations (named):
	  * NewTransition is the CAkMusicSwitchTransition construction retail inlines (malloc 0x48, destination referenced).
	  * SplitRule sets the transition-segment entry rule's jump id to 0 (retail leaves it unset; it is not read there).
	  * GetTransitionRule's "destination is sequence specific" flag starts false (retail reads it uninitialised when no
	    rule matches and the first rule is returned).
	  * ScheduleSwitchTransition: when the window's last valid transition already plays the destination (continue to
	    play) the two transitions just made are disposed; retail breaks out leaking them and their reference on the new
	    destination context (which would keep its switch context, and so CAkMusicRenderer::Destroy's wait, alive). The
	    window walk also stops when the window runs off the end of its chain, where retail would read a NULL item.
	  * -akdiag: every switch change (group, switch, destination node), every scheduled transition (source / destination
	    node, sync type, sync time, look-ahead) and every sync is logged.
=============================================================================*/
#include "AkMusicSwitchCtx.h"
#include "AkMusicSwitchCntr.h"
#include "AkMatrixSequencer.h"
#include "AkScheduleWindow.h"
#include "AkSequenceCtx.h"
#include "AkSegmentCtx.h"
#include "AkMusicSegment.h"
#include "AkPlayingMgr.h"
#include <DishonoredWwiseSilent.h>

// ------------------------------------------------------------------------------------------------------------
// CAkNothingCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x96a4a0, 2013 0x966cc0: the reference its switch context took is dropped. */
void CAkNothingCtx::OnStopped()
{
	AddRef();
	if( m_bWasReferenced )
	{
		m_bWasReferenced = false;
		Release();
	}
	CAkChainCtx::OnStopped();
	Release();
}

// ------------------------------------------------------------------------------------------------------------
// CAkMusicSwitchTransition
// ------------------------------------------------------------------------------------------------------------

CAkMusicSwitchTransition::CAkMusicSwitchTransition( CAkMatrixAwareCtx * in_pDestCtx )
	: pNextItem( NULL )
	, m_pDestCtx( in_pDestCtx )
{
	if( m_pDestCtx )
	{
		m_pDestCtx->AddRef();
	}
	m_cutoffInfo.iCutoffTime = 0;
	m_cutoffInfo.bCutoff = false;
	m_cmdPlay.iRelativeTime = 0;
	m_cmdPlay.fadeParams.transitionTime = 0;
	m_cmdPlay.fadeParams.eFadeCurve = AkCurveInterpolation_Linear;
	m_cmdPlay.fadeParams.iFadeOffset = 0;
	m_cmdStop.iRelativeTime = 0;
	m_cmdStop.transParams.TransitionTime = 0;
	m_cmdStop.transParams.eFadeCurve = AkCurveInterpolation_Linear;
	m_bCmdPlayPending = 0;
	m_bCmdStopPending = 0;
	m_bWasScheduledToStop = 0;
}

/** The construction retail inlines (PrepareFirstContext 0x96b140, ScheduleSwitchTransition 0x96b480). */
CAkMusicSwitchTransition * CAkMusicSwitchCtx::NewTransition( CAkMatrixAwareCtx * in_pDestCtx )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSwitchTransition ) );
	return pMem ? AkPlacementNew( pMem ) CAkMusicSwitchTransition( in_pDestCtx ) : NULL;
}

/** 2012 0x96a4d0, 2013 0x966cf0: the stop the next transition imposes; with a cutoff, nothing of this branch plays past the sync. */
void CAkMusicSwitchTransition::ScheduleToStop( AkInt64 in_iSyncTime, AkInt64 in_iStopTime, AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, bool in_bCutoff )
{
	m_bCmdStopPending = 1;
	m_bWasScheduledToStop = 1;
	m_cmdStop.transParams.TransitionTime = in_iTransDuration;
	m_cmdStop.transParams.eFadeCurve = in_eFadeCurve;
	m_cmdStop.iRelativeTime = in_iStopTime;
	if( in_bCutoff )
	{
		if( m_cutoffInfo.bCutoff )
		{
			if( in_iSyncTime < m_cutoffInfo.iCutoffTime )
			{
				m_cutoffInfo.iCutoffTime = in_iSyncTime;
			}
		}
		else
		{
			m_cutoffInfo.bCutoff = true;
			m_cutoffInfo.iCutoffTime = in_iSyncTime;
		}
	}
}

/** 2012 0x96a880, 2013 0x9670a0: a cancelled stop: a running stop fade is reversed (no longer than its elapsed time, or the max). */
void CAkMusicSwitchTransition::Restore( AkInt64 in_iCurrentTime, AkUInt32 in_uMaxFadeInDuration, bool in_bUseMaxFadeInDuration )
{
	if( !m_bWasScheduledToStop || m_bCmdStopPending )
	{
		m_bCmdStopPending = 0;
		m_bWasScheduledToStop = 0;
		m_cutoffInfo.bCutoff = false;
		return;
	}
	const AkInt32 iElapsed = (AkInt32)in_iCurrentTime - (AkInt32)m_cmdStop.iRelativeTime;
	AkMusicFade revFadeParams;
	if( in_bUseMaxFadeInDuration && iElapsed >= (AkInt32)in_uMaxFadeInDuration )
	{
		revFadeParams.transitionTime = AkTimeConv::SamplesToMilliseconds( (AkInt32)in_uMaxFadeInDuration );
	}
	else
	{
		revFadeParams.transitionTime = AkTimeConv::SamplesToMilliseconds( iElapsed );
	}
	revFadeParams.eFadeCurve = m_cmdStop.transParams.eFadeCurve;
	revFadeParams.iFadeOffset = 0;
	m_pDestCtx->_Play( revFadeParams );
	m_bWasScheduledToStop = 0;
	m_cutoffInfo.bCutoff = false;
}

/** 2012 0x96a900, 2013 0x967120: the destination starts its look-ahead before the sync time; a silent destination does not fade. */
void CAkMusicSwitchTransition::ScheduleToPlay( AkInt64 in_iSyncTime, const AkMusicFade & in_fadeParams, AkInt32 in_iLookAheadTime )
{
	m_pDestCtx->m_iLocalTime = in_iSyncTime;
	m_cmdPlay.iRelativeTime = in_iSyncTime - in_iLookAheadTime;
	m_cmdPlay.fadeParams = in_fadeParams;
	if( !m_pDestCtx->Node() )
	{
		m_cmdPlay.fadeParams.transitionTime = 0;
	}
	if( in_fadeParams.transitionTime > 0 )
	{
		m_cmdPlay.fadeParams.iFadeOffset += in_iLookAheadTime;
	}
	m_bCmdPlayPending = 1;
}

/** 2012 0x96a960, 2013 0x967180: the pending commands falling in the frame, then the destination processes (inside its cutoff). */
void CAkMusicSwitchTransition::Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo )
{
	const AkInt64 iFrameEnd = in_iCurrentTime + in_uNumSamples;
	if( m_bCmdPlayPending && in_iCurrentTime <= m_cmdPlay.iRelativeTime && iFrameEnd > m_cmdPlay.iRelativeTime )
	{
		m_pDestCtx->_Play( m_cmdPlay.fadeParams );
		m_bCmdPlayPending = 0;
	}
	if( m_bCmdStopPending && iFrameEnd > m_cmdStop.iRelativeTime && in_iCurrentTime <= m_cmdStop.iRelativeTime )
	{
		m_pDestCtx->_Stop( m_cmdStop.transParams, (AkUInt32)( m_cmdStop.iRelativeTime - in_iCurrentTime ) );
		m_bCmdStopPending = 0;
	}
	if( ( m_pDestCtx->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !m_pDestCtx->m_bIsPaused )
	{
		if( m_cutoffInfo.bCutoff && ( !in_cutoffInfo.bCutoff || m_cutoffInfo.iCutoffTime < in_cutoffInfo.iCutoffTime ) )
		{
			in_cutoffInfo.bCutoff = true;
			in_cutoffInfo.iCutoffTime = m_cutoffInfo.iCutoffTime;
		}
		m_pDestCtx->Process( in_iCurrentTime, in_uNumSamples, in_cutoffInfo );
	}
}

/** 2012 0x96b240, 2013 0x967a60. */
void CAkMusicSwitchTransition::Dispose()
{
	CAkMatrixAwareCtx * pCtx = m_pDestCtx;
	m_pDestCtx = NULL;
	if( pCtx )
	{
		pCtx->Release();
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x96b290, 2013 0x967ab0. */
void CAkMusicSwitchTransition::Cancel()
{
	m_bCmdPlayPending = 0;
	m_bCmdStopPending = 0;
	m_pDestCtx->_Cancel();
	Dispose();
}

// ------------------------------------------------------------------------------------------------------------
// CAkMusicSwitchCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x96aa40, 2013 0x967260. */
CAkMusicSwitchCtx::CAkMusicSwitchCtx( CAkMusicSwitchCntr * in_pSwitchNode, CAkMusicCtx * in_pParentCtx )
	: CAkMatrixAwareCtx( in_pParentCtx )
	, m_pSwitchCntrNode( in_pSwitchNode )
	, m_targetSwitchID( 0 )
	, m_eGroupType( AkGroupType_Switch )
	, m_delayedSwitchID( 0 )
	, m_itActiveSwitch( NULL )
	, m_bWasReferenced( false )
{
	if( m_pSwitchCntrNode )
	{
		m_pSwitchCntrNode->AddRef();
	}
}

/** 2012 0x96ab50 (the deleting destructor's body). */
CAkMusicSwitchCtx::~CAkMusicSwitchCtx()
{
	if( m_pSwitchCntrNode )
	{
		m_pSwitchCntrNode->Release();
	}
}

/** Retail slot 15 (folded with CAkChainCtx::HandleFatalError: the field at +84). */
CAkMusicNode * CAkMusicSwitchCtx::Node()
{
	return m_pSwitchCntrNode;
}

/** 2012 0x96a530, 2013 0x966d50: a playing switch context stops, fading over the time to its start (retail's arithmetic). */
void CAkMusicSwitchCtx::CancelPlayback( AkInt64 in_iCurrentTime )
{
	AddRef();
	if( m_eStatus == CtxStatusIdle )
	{
		OnStopped();
		Release();
		return;
	}
	if( m_eStatus & CtxStatusPlaying )
	{
		TransParams transParams;
		transParams.TransitionTime = AkTimeConv::SamplesToMilliseconds( (AkInt32)m_iLocalTime - (AkInt32)in_iCurrentTime );
		transParams.eFadeCurve = AkCurveInterpolation_Linear;
		_Stop( transParams, (AkUInt32)-1 );
	}
	Release();
}

/** 2012 0x96a590, 2013 0x966db0: the earliest sync a transition through a transition segment allows. */
AkInt64 CAkMusicSwitchCtx::ComputeWorstMinSyncTime( AkInt64 in_iCurrentTime, const AkMusicTransSrcRule & in_rule1, AkInt32 in_iLookAhead1, CAkMatrixAwareCtx * in_pTransCtx, const AkMusicTransSrcRule & in_rule2, AkInt32 in_iLookAhead2 )
{
	CAkMusicSegment * pFirstSegment = in_pTransCtx->GetFirstSegmentNode();
	const AkInt32 iTransCtxDuration = pFirstSegment ? (AkInt32)pFirstSegment->ActiveDuration() : 0;
	const AkInt32 iSamplesPerMs = (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	AkInt32 iMinLead1 = iSamplesPerMs * in_rule1.fadeParams.transitionTime - in_rule1.fadeParams.iFadeOffset;
	if( iMinLead1 <= in_iLookAhead1 )
	{
		iMinLead1 = in_iLookAhead1;
	}
	const AkInt64 iMinSync1 = in_iCurrentTime + iMinLead1;
	AkInt32 iMinLead2 = iSamplesPerMs * in_rule2.fadeParams.transitionTime - in_rule2.fadeParams.iFadeOffset;
	if( iMinLead2 <= in_iLookAhead2 )
	{
		iMinLead2 = in_iLookAhead2;
	}
	const AkInt64 iMinSync2 = in_iCurrentTime + iMinLead2 - iTransCtxDuration;
	return iMinSync1 <= iMinSync2 ? iMinSync2 : iMinSync1;
}

/**
 * 2012 0x96a640: the rule from what plays in the source window to the destination's first segment; a rule naming a
 * playlist container makes its destination jump to the rule's playlist item. Past 64 look-aheads, the panic rule.
 */
const AkMusicTransitionRule & CAkMusicSwitchCtx::GetTransitionRule( const CAkScheduleWindow & in_source, CAkMatrixAwareCtx *& io_pDestContext, AkUInt32 & io_uSrcSegmentLookAhead )
{
	if( ++io_uSrcSegmentLookAhead > 64 )
	{
		return CAkMusicTransAware::GetPanicTransitionRule();
	}
	CAkMusicNode * pSrcParentNode = NULL;
	CAkMusicSegment * pSrcNode = in_source.GetNode( &pSrcParentNode );
	const AkUniqueID srcID = pSrcNode ? pSrcNode->key : 0;
	CAkScheduleWindow destination( io_pDestContext, true );
	CAkMusicNode * pDestParentNode = NULL;
	CAkMusicSegment * pDestNode = destination.GetNode( &pDestParentNode );
	const AkUniqueID destID = pDestNode ? pDestNode->key : 0;
	bool bIsDestSequenceSpecific = false;
	const AkMusicTransitionRule & rule = m_pSwitchCntrNode->GetTransitionRule( m_pSwitchCntrNode, srcID, pSrcParentNode, destID, pDestParentNode, bIsDestSequenceSpecific );
	if( bIsDestSequenceSpecific )
	{
		CAkSequenceCtx * pSequenceCtx = static_cast<CAkSequenceCtx *>( destination.m_itScheduledItem.pCtx );
		pSequenceCtx->AddRef();
		if( !pSequenceCtx->JumpToSegment( rule.destRule.uJumpToID ) )
		{
			io_pDestContext->_Cancel();
			io_pDestContext = NULL;
		}
		pSequenceCtx->Release();
	}
	return rule;
}

/** 2012 0x96a720, 2013 0x966f40: the rule as source / destination halves, and the halves to and from a transition segment. */
void CAkMusicSwitchCtx::SplitRule( const AkMusicTransitionRule & in_rule, AkMusicTransSrcRule & out_ruleFrom, AkMusicTransDestRule & out_ruleTo, AkMusicTransDestRule & out_ruleToTrans, AkMusicTransSrcRule & out_ruleFromTrans )
{
	out_ruleFrom = in_rule.srcRule;
	out_ruleTo = in_rule.destRule;
	const AkMusicTransitionObject * pTransObj = in_rule.pTransObj;
	if( pTransObj )
	{
		out_ruleToTrans.fadeParams = pTransObj->fadeInParams;
		out_ruleToTrans.uCueFilterHash = 0;
		out_ruleToTrans.uJumpToID = 0;
		out_ruleToTrans.eEntryType = EntryTypeEntryMarker;
		out_ruleToTrans.bPlayPreEntry = pTransObj->bPlayPreEntry;
		out_ruleToTrans.bDestMatchSourceCueName = 0;
		out_ruleFromTrans.fadeParams = pTransObj->fadeOutParams;
		out_ruleFromTrans.uCueFilterHash = 0;
		out_ruleFromTrans.eSyncType = SyncTypeExitMarker;
		out_ruleFromTrans.bPlayPostExit = pTransObj->bPlayPostExit;
	}
	else
	{
		out_ruleToTrans = out_ruleTo;
		out_ruleFromTrans = out_ruleFrom;
	}
}

/** 2012 0x96aad0, 2013 0x9672f0: the first destination's preparation. */
AkInt32 CAkMusicSwitchCtx::Prepare( const AkMusicTransDestRule * in_pRule, AkInt32 in_iMinStartPosition, AkUInt32 & out_uSelectedCue, AkUInt32 in_uCueHashForMatchSrc )
{
	return m_queueTransitions.First()->m_pDestCtx->Prepare( in_pRule, in_iMinStartPosition, out_uSelectedCue, in_uCueHashForMatchSrc );
}

/** 2012 0x96aae0, 2013 0x967300. */
bool CAkMusicSwitchCtx::CanRestartPlaying()
{
	return ( ( m_eStatus & CtxStatusPlaying ) || m_eStatus == CtxStatusIdle ) && m_queueTransitions.Last()->m_pDestCtx->CanRestartPlaying();
}

/** 2012 0x96ab00, 2013 0x967320: the first destination starts with this context. */
void CAkMusicSwitchCtx::OnPlayed()
{
	if( m_eStatus == CtxStatusIdle )
	{
		m_queueTransitions.First()->m_pDestCtx->m_iLocalTime = 0;
		AkMusicFade fadeParams;
		fadeParams.transitionTime = 0;
		fadeParams.eFadeCurve = AkCurveInterpolation_Linear;
		fadeParams.iFadeOffset = 0;
		m_queueTransitions.First()->m_pDestCtx->_Play( fadeParams );
	}
	CAkMusicCtx::OnPlayed();
}

/** 2012 0x96abb0, 2013 0x9673d0. */
void CAkMusicSwitchCtx::EndInit()
{
	for( CAkMusicSwitchTransition * pTrans = m_queueTransitions.First(); pTrans; pTrans = pTrans->pNextItem )
	{
		if( pTrans->m_pDestCtx )
		{
			pTrans->m_pDestCtx->EndInit();
		}
	}
}

/** 2012 0x96abd0, 2013 0x9673f0: the earliest play / audible times over the destinations (a top level skips its silent first one). */
void CAkMusicSwitchCtx::QueryLookAheadInfo( AkInt64 & out_iPlayTime, AkInt64 & out_iPlayTimeAudible )
{
	CAkMusicSwitchTransition * pTrans = m_queueTransitions.First();
	if( !pTrans )
	{
		out_iPlayTime = 0;
		out_iPlayTimeAudible = 0;
		return;
	}
	if( !m_pParentCtx && pTrans->pNextItem )
	{
		pTrans = pTrans->pNextItem;
	}
	AkInt64 iPlayTime, iPlayTimeAudible;
	pTrans->m_pDestCtx->QueryLookAheadInfo( iPlayTime, iPlayTimeAudible );
	out_iPlayTime = iPlayTime + pTrans->m_pDestCtx->m_iLocalTime;
	out_iPlayTimeAudible = iPlayTimeAudible + pTrans->m_pDestCtx->m_iLocalTime;
	for( CAkMusicSwitchTransition * pNext = pTrans->pNextItem; pNext; pNext = pNext->pNextItem )
	{
		pNext->m_pDestCtx->QueryLookAheadInfo( iPlayTime, iPlayTimeAudible );
		iPlayTime += pNext->m_pDestCtx->m_iLocalTime;
		if( iPlayTime < out_iPlayTime )
		{
			out_iPlayTime = iPlayTime;
		}
		iPlayTimeAudible += pNext->m_pDestCtx->m_iLocalTime;
		if( iPlayTimeAudible < out_iPlayTimeAudible )
		{
			out_iPlayTimeAudible = iPlayTimeAudible;
		}
	}
}

/**
 * 2012 0x96acd0: at this switch level the window follows its branch (the active transition the first time), through
 * the following transitions whose destinations start before the window; the window ends where the next one starts.
 */
void CAkMusicSwitchCtx::GetNextScheduleWindow( CAkScheduleWindow & io_window, bool in_bDoNotGrow )
{
	CAkScheduleWindow::AutoBranchLevel autoBranchLevel( io_window );
	CAkMusicSwitchTransition * itBranch = io_window.GetBranch();
	if( !itBranch )
	{
		if( m_itActiveSwitch )
		{
			itBranch = m_itActiveSwitch;
			io_window.SetBranch( itBranch );
		}
		else if( !io_window.m_itScheduledItem.pItem )
		{
			return;
		}
	}
	itBranch->m_pDestCtx->GetNextScheduleWindow( io_window, in_bDoNotGrow );
	CAkMusicSwitchTransition * itNextBranch = itBranch->pNextItem;
	if( !itNextBranch )
	{
		return;
	}
	AkInt64 iNextBranchTime = itNextBranch->m_pDestCtx->m_iLocalTime;
	while( io_window.StartTimeRelativeToCurrentLevel() >= iNextBranchTime )
	{
		io_window.SetBranch( itNextBranch );
		itNextBranch->m_pDestCtx->GetNextScheduleWindow( io_window, in_bDoNotGrow );
		itNextBranch = itNextBranch->pNextItem;
		if( !itNextBranch )
		{
			return;
		}
		iNextBranchTime = itNextBranch->m_pDestCtx->m_iLocalTime;
	}
	const AkUInt64 uDuration = (AkUInt64)( iNextBranchTime - io_window.StartTimeRelativeToCurrentLevel() );
	if( io_window.m_bIsDurationInfinite || io_window.m_uWindowDuration > uDuration )
	{
		io_window.SetDuration( uDuration, false );
	}
}

/** 2012 0x96adf0, 2013 0x967610. */
void CAkMusicSwitchCtx::RefreshWindow( CAkScheduleWindow & io_window )
{
	CAkScheduleWindow::AutoBranchLevel autoBranchLevel( io_window );
	CAkMusicSwitchTransition * itBranch = io_window.GetBranch();
	itBranch->m_pDestCtx->RefreshWindow( io_window );
	CAkMusicSwitchTransition * itNextBranch = itBranch->pNextItem;
	if( itNextBranch )
	{
		const AkUInt64 uDuration = (AkUInt64)( itNextBranch->m_pDestCtx->m_iLocalTime - io_window.StartTimeRelativeToCurrentLevel() );
		if( io_window.m_bIsDurationInfinite || io_window.m_uWindowDuration > uDuration )
		{
			io_window.SetDuration( uDuration, false );
		}
	}
}

/**
 * 2012 0x96ae70: the queue's last transition stops at the sync point (fading by the source rule, or past the
 * post-exit at an exit cue; a stop already running is reversed first), the new one plays at it, the actions after
 * it on the source's items are cancelled, and the new transition is queued.
 */
void CAkMusicSwitchCtx::ScheduleTransition( AkInt64 in_iCurrentTime, CAkMusicSwitchTransition * in_pNewTransition, const AkMusicTransSrcRule & in_ruleFrom, const AkMusicTransDestRule & in_ruleTo, CAkScheduleWindow & io_source, AkInt32 in_iDestinationLookAhead, AkInt64 in_iSyncPoint, bool in_bIsFromTransitionCtx, AkListBareLight<AkAssociatedAction> & io_listCancelledActions )
{
	CAkMusicSegment * pSrcSegmentNode = io_source.GetNode( NULL );
	CAkMusicSwitchTransition * pLast = m_queueTransitions.Last();
	if( !pLast->m_bWasScheduledToStop || pLast->m_bCmdStopPending )
	{
		pLast->m_bCmdStopPending = 0;
	}
	else
	{
		const AkInt32 iElapsed = (AkInt32)in_iCurrentTime - (AkInt32)pLast->m_cmdStop.iRelativeTime;
		AkMusicFade fadeParams;
		if( iElapsed < (AkInt32)in_iSyncPoint - (AkInt32)in_iCurrentTime )
		{
			fadeParams.transitionTime = AkTimeConv::SamplesToMilliseconds( iElapsed );
		}
		else
		{
			fadeParams.transitionTime = AkTimeConv::SamplesToMilliseconds( (AkInt32)( in_iSyncPoint - in_iCurrentTime ) );
		}
		fadeParams.eFadeCurve = pLast->m_cmdStop.transParams.eFadeCurve;
		fadeParams.iFadeOffset = 0;
		pLast->m_pDestCtx->_Play( fadeParams );
	}
	pLast->m_bWasScheduledToStop = 0;
	pLast->m_cutoffInfo.bCutoff = false;
	AkInt64 iStopTime = in_iSyncPoint;
	if( in_ruleFrom.fadeParams.transitionTime > 0 && pSrcSegmentNode )
	{
		iStopTime = in_iSyncPoint - (AkInt32)( in_ruleFrom.fadeParams.transitionTime * (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 ) - in_ruleFrom.fadeParams.iFadeOffset );
	}
	else if( in_ruleFrom.bPlayPostExit && io_source.IsAtExitCue( (AkUInt64)in_iSyncPoint ) )
	{
		iStopTime = in_iSyncPoint + pSrcSegmentNode->PostExitDuration();
	}
	pLast->ScheduleToStop( in_iSyncPoint, iStopTime, in_ruleFrom.fadeParams.transitionTime, in_ruleFrom.fadeParams.eFadeCurve, !in_bIsFromTransitionCtx );
	io_source.m_itScheduledItem.pItem->ForcePostExit( in_ruleFrom.bPlayPostExit != 0 );
	in_pNewTransition->ScheduleToPlay( in_iSyncPoint, in_ruleTo.fadeParams, in_iDestinationLookAhead );
	io_source.CancelActionsAfterTransitionSyncPoint( io_listCancelledActions, in_iSyncPoint );
	while( !io_source.m_bIsDurationInfinite )
	{
		GetNextScheduleWindow( io_source, true );
		io_source.CancelActionsAfterTransitionSyncPoint( io_listCancelledActions, in_iSyncPoint );
	}
	m_queueTransitions.AddLast( in_pNewTransition );
	AKDIAG( L"akdiag: music transition scheduled in switch 0x%08x: from segment 0x%08x (sync type %u, stop %lld) to node 0x%08x at %lld (look-ahead %d, fade-in %d ms)%s",
		m_pSwitchCntrNode->key, pSrcSegmentNode ? pSrcSegmentNode->key : 0, (AkUInt32)in_ruleFrom.eSyncType, iStopTime,
		in_pNewTransition->m_pDestCtx->Node() ? in_pNewTransition->m_pDestCtx->Node()->key : 0, in_iSyncPoint, in_iDestinationLookAhead, in_ruleTo.fadeParams.transitionTime,
		in_bIsFromTransitionCtx ? L" (after a transition segment)" : L"" );
}

/** 2012 0x96b040, 2013 0x967860. */
bool CAkMusicSwitchCtx::HasOrAscendentHasPendingTransition()
{
	if( m_itActiveSwitch->pNextItem )
	{
		return true;
	}
	for( CAkMusicSwitchCtx * pParent = static_cast<CAkMusicSwitchCtx *>( m_pParentCtx ); pParent; pParent = static_cast<CAkMusicSwitchCtx *>( pParent->m_pParentCtx ) )
	{
		if( pParent->m_itActiveSwitch->pNextItem )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x96b060, 2013 0x967880: the last transition a new switch change cannot cancel (one already playing or not restartable). */
CAkMusicSwitchTransition * CAkMusicSwitchCtx::FindLastNonCancellableTransition()
{
	CAkMusicSwitchTransition * pTrans = m_itActiveSwitch;
	CAkMusicSwitchTransition * pLastNonCancellable = pTrans;
	for( CAkMusicSwitchTransition * pNext = pTrans->pNextItem; pNext; pNext = pNext->pNextItem )
	{
		if( ( pNext->m_pDestCtx->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !pNext->m_pDestCtx->m_bIsPaused )
		{
			pLastNonCancellable = pTrans;
		}
		if( !pTrans->m_pDestCtx->CanRestartPlaying() )
		{
			pLastNonCancellable = pNext;
		}
		pTrans = pNext;
	}
	return pLastNonCancellable;
}

/** 2012 0x96b0a0, 2013 0x9678c0: the window is moved onto that transition's destination at this level. */
void CAkMusicSwitchCtx::MoveWindowToLastNonCancellableTransition( CAkMusicSwitchTransition * in_pLastNonCancellableTransition, CAkScheduleWindow & io_window )
{
	CAkScheduleWindow::AutoBranchLevel autoBranchLevel( io_window );
	CAkMusicSwitchTransition * it = m_queueTransitions.First();
	while( it && it != in_pLastNonCancellableTransition )
	{
		it = it->pNextItem;
	}
	if( io_window.GetBranch() != it )
	{
		io_window.SetBranch( it );
		it->m_pDestCtx->GetNextScheduleWindow( io_window, false );
	}
}

/** 2012 0x96b100, 2013 0x967920: once nothing is pending here or above, the playing destinations apply their delayed switches. */
void CAkMusicSwitchCtx::TryPropagateDelayedSwitchChange()
{
	if( m_itActiveSwitch->pNextItem )
	{
		return;
	}
	CAkMusicSwitchCtx * pParent = static_cast<CAkMusicSwitchCtx *>( m_pParentCtx );
	if( pParent && pParent->HasOrAscendentHasPendingTransition() )
	{
		return;
	}
	for( CAkMusicSwitchTransition * pTrans = m_queueTransitions.First(); pTrans; pTrans = pTrans->pNextItem )
	{
		CAkMatrixAwareCtx * pCtx = pTrans->m_pDestCtx;
		if( pCtx && ( pCtx->m_eStatus & CtxStatusPlaying ) )
		{
			pCtx->PerformDelayedSwitchChange();
		}
	}
}

/** 2012 0x96b140, 2013 0x967960: the first destination becomes the active transition, prepared from its start. */
AKRESULT CAkMusicSwitchCtx::PrepareFirstContext( CAkMatrixAwareCtx * in_pCtx )
{
	CAkMusicSwitchTransition * pTrans = NewTransition( in_pCtx );
	if( !pTrans )
	{
		return AK_Fail;
	}
	m_queueTransitions.AddLast( pTrans );
	m_itActiveSwitch = m_queueTransitions.First();
	AkUInt32 uSelectedCue;
	Prepare( NULL, 0, uSelectedCue, 0 );
	return AK_Success;
}

/** 2012 0x96b1c0, 2013 0x9679e0: every transition after in_pLastValidTransition leaves the queue for the cancelled list. */
void CAkMusicSwitchCtx::DequeueCancellableTransitions( CAkMusicSwitchTransition * in_pLastValidTransition, TransQueue & out_listCancelledTransitions )
{
	CAkMusicSwitchTransition * pValid = m_queueTransitions.First();
	while( pValid && pValid != in_pLastValidTransition )
	{
		pValid = pValid->pNextItem;
	}
	CAkMusicSwitchTransition * pTrans = pValid->pNextItem;
	while( pTrans )
	{
		CAkMusicSwitchTransition * pNext = pTrans->pNextItem;
		if( pTrans == m_queueTransitions.m_pFirst )
		{
			m_queueTransitions.m_pFirst = pNext;
		}
		else
		{
			pValid->pNextItem = pNext;
		}
		if( pTrans == m_queueTransitions.m_pLast )
		{
			m_queueTransitions.m_pLast = pValid;
		}
		--m_queueTransitions.m_ulNumListItems;
		out_listCancelledTransitions.AddLast( pTrans );
		pTrans = pNext;
	}
}

/** 2012 0x96b2e0, 2013 0x967b00: the switch subscription and every transition go; the self reference is dropped. */
void CAkMusicSwitchCtx::OnStopped()
{
	AddRef();
	UnsubscribeSwitches();
	while( CAkMusicSwitchTransition * pTrans = m_queueTransitions.RemoveFirst() )
	{
		pTrans->Dispose();
	}
	m_itActiveSwitch = m_queueTransitions.First();
	m_queueTransitions.RemoveAll();
	if( m_bWasReferenced )
	{
		m_bWasReferenced = false;
		Release();
	}
	CAkMatrixAwareCtx::OnStopped();
	Release();
}

/** 2012 0x96b3a0, 2013 0x967bc0: a transition reaches its sync time and becomes the active one (AK_MusicSyncPoint if registered). */
void CAkMusicSwitchCtx::Sync( CAkMusicSwitchTransition * in_itNewTransition )
{
	m_itActiveSwitch = in_itNewTransition;
	if( m_pSequencer->GetMusicSyncFlags() & AK_MusicSyncPoint )
	{
		CAkScheduleWindow curWindow( this, false );
		if( curWindow.m_itScheduledItem.pItem )
		{
			CAkMusicNode * pParentNode = NULL;
			CAkMusicNode * pNode = curWindow.GetNode( &pParentNode );
			if( !pNode )
			{
				pNode = pParentNode;
			}
			g_pPlayingMgr->NotifyMusic( m_pSequencer->m_UserParams.m_PlayingID, AK_MusicSyncPoint, pNode->GetMusicGrid() );
		}
	}
	AKDIAG( L"akdiag: music switch 0x%08x synced to node 0x%08x at sequencer time %llu", m_pSwitchCntrNode->key, m_itActiveSwitch->m_pDestCtx->Node() ? m_itActiveSwitch->m_pDestCtx->Node()->key : 0, m_pSequencer->m_uTime );
	TryPropagateDelayedSwitchChange();
}

/** 2012 0x96b430, 2013 0x967c50: a context of the node, child of this one. */
CAkMatrixAwareCtx * CAkMusicSwitchCtx::CreateMusicContext( AkUniqueID in_ID )
{
	CAkMusicNode * pNode = static_cast<CAkMusicNode *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ID ) );
	if( !pNode )
	{
		return NULL;
	}
	CAkMatrixAwareCtx * pCtx = pNode->CreateContext( this, m_pSequencer->m_pGameObj, m_pSequencer->m_UserParams );
	pNode->Release();
	return pCtx;
}

/** 2012 0x96bee0, 2013 0x968700: the node's context, or a silent "nothing" chain (no node, or one that could not be created). */
CAkMatrixAwareCtx * CAkMusicSwitchCtx::CreateDestinationContext( AkUniqueID in_ID )
{
	if( in_ID )
	{
		CAkMatrixAwareCtx * pCtx = CreateMusicContext( in_ID );
		if( pCtx )
		{
			return pCtx;
		}
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkNothingCtx ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkNothingCtx * pNothing = AkPlacementNew( pMem ) CAkNothingCtx( this );
	pNothing->AddRef();
	if( pNothing->CAkMatrixAwareCtx::Init( m_pSequencer->m_pGameObj, m_pSequencer->m_UserParams ) == AK_Success && pNothing->EnqueueItem( 0, NULL ) )
	{
		pNothing->AddRef();
		pNothing->m_bWasReferenced = true;
		pNothing->Release();
		return pNothing;
	}
	pNothing->_Cancel();
	pNothing->Release();
	return NULL;
}

/**
 * 2012 0x96b480: schedules the transition to the destination context. Past the last transition it cannot cancel, it
 * walks the source windows: the rule (and transition segment) from the window's segment, the destination's look-ahead,
 * the earliest sync time that leaves room for the fades and look-ahead, then the sync point (for "same time" and
 * matched-cue entries the destination is re-prepared at the matching position and the sync re-checked). Without a sync
 * point in a window it moves to the next one. Switching back to what already plays (continue to play) restores it.
 */
void CAkMusicSwitchCtx::ScheduleSwitchTransition( AkUniqueID in_destinationID, CAkMatrixAwareCtx *& io_pNewContext )
{
	CAkScheduleWindow wndSource( this, false );
	if( !wndSource.m_itScheduledItem.pItem )
	{
		return;
	}
	AkInt64 iLocalTime = m_iLocalTime;
	if( m_pParentCtx )
	{
		iLocalTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
	}
	const AkInt64 iCurrentTime = (AkInt64)m_pSequencer->m_uTime - iLocalTime;
	CAkMusicSwitchTransition * pLastNonCancellable = FindLastNonCancellableTransition();
	TransQueue listCancelledTransitions;
	bool bNoTransitionScheduled = true;
	AkListBareLight<AkAssociatedAction> listCancelledActions;
	if( m_pSwitchCntrNode->m_bIsContinuePlayback )
	{
		CAkMusicNode * pNode = pLastNonCancellable->m_pDestCtx->Node();
		if( pNode ? pNode->key == in_destinationID : in_destinationID == 0 )
		{
			DequeueCancellableTransitions( pLastNonCancellable, listCancelledTransitions );
			io_pNewContext->_Cancel();
			io_pNewContext = NULL;
			if( m_queueTransitions.Last()->m_pDestCtx->CanRestartPlaying() )
			{
				m_queueTransitions.Last()->Restore( iCurrentTime, 0, false );
			}
			AKDIAG( L"akdiag: music switch 0x%08x: node 0x%08x continues to play", m_pSwitchCntrNode->key, in_destinationID );
			goto cleanup;
		}
	}
	MoveWindowToLastNonCancellableTransition( pLastNonCancellable, wndSource );
	{
		AkUInt32 uSrcSegmentLookAhead = 0;
		while( wndSource.m_itScheduledItem.pItem && !wndSource.m_itScheduledItem.pItem->m_bWasPlaybackSkipped )
		{
			const AkMusicTransitionRule & rule = GetTransitionRule( wndSource, io_pNewContext, uSrcSegmentLookAhead );
			if( !io_pNewContext )
			{
				break;
			}
			CAkMusicSwitchTransition * pNewTransition = NULL;
			CAkMusicSwitchTransition * pTransitionToDest = NULL;
			CAkMatrixAwareCtx * pTransCtx = NULL;
			if( rule.pTransObj )
			{
				CAkMusicNode * pTransNode = static_cast<CAkMusicNode *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( rule.pTransObj->segmentID ) );
				if( pTransNode )
				{
					pTransCtx = pTransNode->CreateContext( this, m_pSequencer->m_pGameObj, m_pSequencer->m_UserParams );
					pTransNode->Release();
				}
				if( pTransCtx )
				{
					pNewTransition = NewTransition( pTransCtx );
					pTransitionToDest = NewTransition( io_pNewContext );
					if( !pNewTransition || !pTransitionToDest )
					{
						if( pNewTransition )
						{
							pNewTransition->Cancel();
						}
						else
						{
							pTransCtx->_Cancel();
						}
						pTransCtx = NULL;
						if( pTransitionToDest )
						{
							pTransitionToDest->Dispose();
							pTransitionToDest = NULL;
						}
						pNewTransition = NULL;
					}
				}
			}
			if( !pNewTransition )
			{
				pNewTransition = NewTransition( io_pNewContext );
				if( !pNewTransition )
				{
					break;
				}
			}
			AkMusicTransSrcRule ruleFrom, ruleFromTrans;
			AkMusicTransDestRule ruleTo, ruleToTrans;
			SplitRule( rule, ruleFrom, ruleTo, ruleToTrans, ruleFromTrans );
			AkUInt32 uDestCueHash = 0;
			AkInt32 iDestLookAhead = io_pNewContext->Prepare( &ruleTo, 0, uDestCueHash, 0 );
			AkInt64 iSyncPoint2 = 0;
			CAkScheduleWindow wndTrans( true );
			AkInt32 iTransLookAhead;
			AkInt64 iMinSyncTime;
			AkInt64 iMinSyncTime2;
			const AkInt32 iSamplesPerMs = (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
			if( pTransitionToDest )
			{
				AkUInt32 uTransCtxDestCueHash = 0;
				iTransLookAhead = pTransCtx->Prepare( &ruleToTrans, 0, uTransCtxDestCueHash, 0 );
				iMinSyncTime = ComputeWorstMinSyncTime( iCurrentTime, ruleFrom, iTransLookAhead, pTransCtx, ruleFromTrans, iDestLookAhead );
				AkInt32 iMinLead = ruleFromTrans.fadeParams.transitionTime * iSamplesPerMs - ruleFromTrans.fadeParams.iFadeOffset;
				if( iMinLead <= iDestLookAhead )
				{
					iMinLead = iDestLookAhead;
				}
				iMinSyncTime2 = iCurrentTime + iMinLead;
			}
			else
			{
				AkInt32 iMinLead = ruleFrom.fadeParams.transitionTime * iSamplesPerMs - ruleFrom.fadeParams.iFadeOffset;
				iTransLookAhead = iDestLookAhead;
				if( iMinLead <= iDestLookAhead )
				{
					iMinLead = iDestLookAhead;
				}
				iMinSyncTime2 = 0;
				iMinSyncTime = iCurrentTime + iMinLead;
			}
			AkUInt32 uTransCtxCueFilterHash = 0;
			const AkSyncType eSyncType = (AkSyncType)ruleFrom.eSyncType;
			const bool bEntryDependsOnSource = ruleTo.eEntryType == EntryTypeSameTime || ( ruleTo.bDestMatchSourceCueName && ( ruleTo.eEntryType == EntryTypeRandomMarker || ruleTo.eEntryType == EntryTypeRandomUserMarker ) );
			AKRESULT eResult;
			AkInt64 iSyncPoint = 0;
			bool bSchedule = false;
			for( ;; )
			{
				AkUInt32 uSrcCueFilterHash = ruleFrom.uCueFilterHash;
				eResult = wndSource.FindSyncPoint( iMinSyncTime, eSyncType, uSrcCueFilterHash, bEntryDependsOnSource, true, iSyncPoint );
				if( pTransitionToDest && eResult == AK_Success )
				{
					pNewTransition->m_pDestCtx->m_iLocalTime = iSyncPoint;
					{
						CAkScheduleWindow::AutoBranchLevel autoBranchLevel( wndTrans );
						wndTrans.SetBranch( pNewTransition );
						pNewTransition->m_pDestCtx->GetNextScheduleWindow( wndTrans, false );
					}
					eResult = wndTrans.FindSyncPoint( iMinSyncTime2, (AkSyncType)ruleFromTrans.eSyncType, uTransCtxCueFilterHash, true, true, iSyncPoint2 );
				}
				if( !bEntryDependsOnSource )
				{
					bSchedule = eResult == AK_Success;
					break;
				}
				if( eResult != AK_Success )
				{
					break;
				}
				if( !wndSource.m_itScheduledItem.pItem->m_pSegment )
				{
					bSchedule = true;
					break;
				}
				AkInt32 iLookAhead;
				if( ruleTo.eEntryType == EntryTypeSameTime )
				{
					const AkInt64 iSegmentPosition = wndSource.m_itScheduledItem.CtxTimeToSegmentPosition( iSyncPoint - wndSource.ChainCtxTimeRelativeToLevel( 0 ) );
					iLookAhead = io_pNewContext->Prepare( &ruleTo, (AkInt32)iSegmentPosition, uDestCueHash, 0 );
				}
				else
				{
					iLookAhead = io_pNewContext->Prepare( &ruleTo, 0, uDestCueHash, uSrcCueFilterHash );
				}
				iDestLookAhead = iLookAhead;
				if( pTransitionToDest )
				{
					CAkMusicSegment * pFirstSegment = pTransCtx->GetFirstSegmentNode();
					iLookAhead = pFirstSegment ? iDestLookAhead - (AkInt32)pFirstSegment->ActiveDuration() : iDestLookAhead;
					if( iLookAhead < iTransLookAhead )
					{
						iLookAhead = iTransLookAhead;
					}
				}
				else
				{
					iTransLookAhead = iLookAhead;
				}
				if( iLookAhead <= iSyncPoint - iCurrentTime )
				{
					bSchedule = true;
					break;
				}
				iMinSyncTime = iCurrentTime + iLookAhead;
			}
			if( !bSchedule )
			{
				if( pTransitionToDest )
				{
					pNewTransition->Cancel();
					pTransitionToDest->Dispose();
				}
				else
				{
					pNewTransition->Dispose();
				}
				GetNextScheduleWindow( wndSource, false );
				continue;
			}
			CAkMusicSwitchTransition * pLastValid;
			{
				CAkScheduleWindow::AutoBranchLevel autoBranchLevel( wndSource );
				pLastValid = wndSource.GetBranch();
			}
			DequeueCancellableTransitions( pLastValid, listCancelledTransitions );
			if( m_pSwitchCntrNode->m_bIsContinuePlayback )
			{
				CAkMusicNode * pNode = pLastValid->m_pDestCtx->Node();
				if( pNode ? pNode->key == in_destinationID : in_destinationID == 0 )
				{
					if( pTransitionToDest )
					{
						pNewTransition->Cancel();
						pTransitionToDest->Dispose();
					}
					else
					{
						pNewTransition->Dispose();
					}
					break;
				}
			}
			bNoTransitionScheduled = false;
			RefreshWindow( wndSource );
			ScheduleTransition( iCurrentTime, pNewTransition, ruleFrom, ruleToTrans, wndSource, iTransLookAhead, iSyncPoint, false, listCancelledActions );
			if( pTransitionToDest )
			{
				ScheduleTransition( iCurrentTime, pTransitionToDest, ruleFromTrans, ruleTo, wndTrans, iDestLookAhead, iSyncPoint2, true, listCancelledActions );
			}
			break;
		}
	}
	if( bNoTransitionScheduled && io_pNewContext )
	{
		io_pNewContext->_Cancel();
		io_pNewContext = NULL;
		if( m_queueTransitions.Last()->m_pDestCtx->CanRestartPlaying() )
		{
			m_queueTransitions.Last()->Restore( iCurrentTime, 0, false );
		}
	}
	m_pSequencer->RescheduleCancelledActions( listCancelledActions );
cleanup:
	while( CAkMusicSwitchTransition * pTrans = listCancelledTransitions.RemoveFirst() )
	{
		pTrans->Cancel();
	}
}

/** 2012 0x96bd40, 2013 0x968560: the transitions process the frame; a transition reaching its destination's start syncs; idle ones go. */
void CAkMusicSwitchCtx::Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo )
{
	const AkInt64 iCurrentTime = in_iCurrentTime - m_iLocalTime;
	in_cutoffInfo.iCutoffTime -= m_iLocalTime;
	AddRef();
	AkUInt32 uNumSamples = in_uNumSamples;
	if( ( m_eStatus & CtxStatusLastFrame ) && m_uNumLastSamples != (AkUInt32)-1 )
	{
		uNumSamples = m_uNumLastSamples;
	}
	const AkInt64 iCurrentFrameEnd = iCurrentTime + uNumSamples;
	CAkMusicSwitchTransition * pPrev = NULL;
	CAkMusicSwitchTransition * pTrans = m_queueTransitions.First();
	while( pTrans )
	{
		const AkInt64 iDestStart = pTrans->m_pDestCtx->m_iLocalTime;
		if( iCurrentTime <= iDestStart && iCurrentFrameEnd > iDestStart )
		{
			Sync( pTrans );
		}
		CAkMatrixAwareCtx * pDest = pTrans->m_pDestCtx;
		if( pTrans->m_bCmdStopPending || ( ( pDest->m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) && !pDest->m_bIsPaused ) || pTrans->m_bCmdPlayPending )
		{
			AkCutoffInfo cutoffInfo = in_cutoffInfo;
			pTrans->Process( iCurrentTime, uNumSamples, cutoffInfo );
		}
		else if( pTrans != m_itActiveSwitch )
		{
			CAkMusicSwitchTransition * pNext = pTrans->pNextItem;
			if( pTrans == m_queueTransitions.m_pFirst )
			{
				m_queueTransitions.m_pFirst = pNext;
			}
			else
			{
				pPrev->pNextItem = pNext;
			}
			if( pTrans == m_queueTransitions.m_pLast )
			{
				m_queueTransitions.m_pLast = pPrev;
			}
			--m_queueTransitions.m_ulNumListItems;
			pTrans->Dispose();
			pTrans = pNext;
			continue;
		}
		pPrev = pTrans;
		pTrans = pTrans->pNextItem;
	}
	ProcessEpilogue( iCurrentTime, uNumSamples );
}

/** 2012 0x96bf90, 2013 0x9687b0: the switch's node (or nothing) becomes the destination of a new transition. */
void CAkMusicSwitchCtx::PerformSwitchChange( AkUInt32 in_switchID )
{
	m_delayedSwitchID = 0;
	if( m_eStatus > CtxStatusPlaying )
	{
		return;
	}
	AkUniqueID nodeID = 0;
	m_pSwitchCntrNode->GetSwitchNode( in_switchID, nodeID );
	AKDIAG( L"akdiag: music switch 0x%08x: %s group 0x%08x -> 0x%08x selects node 0x%08x (sequencer time %llu)", m_pSwitchCntrNode->key,
		m_eGroupType == AkGroupType_State ? L"state" : L"switch", m_pSwitchCntrNode->m_ulGroupID, in_switchID, nodeID, m_pSequencer ? m_pSequencer->m_uTime : 0 );
	CAkMatrixAwareCtx * pNewContext = CreateDestinationContext( nodeID );
	if( pNewContext )
	{
		ScheduleSwitchTransition( nodeID, pNewContext );
		if( pNewContext )
		{
			pNewContext->EndInit();
		}
	}
}

/**
 * 2012 0x96bff0: subscribes to the group; a child context starts on the current switch's node, a top-level one on
 * silence followed at once by a switch change to the current switch's node.
 */
AKRESULT CAkMusicSwitchCtx::Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams, AkUInt32 in_ulGroupID, AkGroupType in_eGroupType )
{
	m_eGroupType = in_eGroupType;
	AKRESULT eResult = CAkMatrixAwareCtx::Init( in_GameObject, in_rUserparams );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = SubscribeSwitch( in_ulGroupID, in_eGroupType );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	AddRef();
	m_bWasReferenced = true;
	const AkUInt32 switchToUse = GetSwitchToUse( m_pSequencer->m_pGameObj, in_ulGroupID, in_eGroupType );
	if( m_pParentCtx )
	{
		AkUniqueID nodeID = 0;
		m_pSwitchCntrNode->GetSwitchNode( switchToUse, nodeID );
		CAkMatrixAwareCtx * pDestination = CreateDestinationContext( nodeID );
		if( !pDestination || PrepareFirstContext( pDestination ) != AK_Success )
		{
			eResult = AK_Fail;
		}
	}
	else
	{
		CAkMatrixAwareCtx * pDestination = CreateDestinationContext( 0 );
		if( pDestination && PrepareFirstContext( pDestination ) == AK_Success )
		{
			PerformSwitchChange( switchToUse );
			m_targetSwitchID = switchToUse;
			return eResult;
		}
		eResult = AK_Fail;
	}
	m_targetSwitchID = switchToUse;
	return eResult;
}

/**
 * 2012 0x96c0d0: a new switch / state of the group for this context (a switch only on its own game object); while a
 * transition is pending here or above, it is kept as the delayed switch.
 */
void CAkMusicSwitchCtx::SetSwitch( AkUInt32 in_Switch, CAkRegisteredObj * in_pGameObj )
{
	if( ( in_pGameObj != m_pSequencer->m_pGameObj && m_eGroupType != AkGroupType_State ) || m_targetSwitchID == in_Switch )
	{
		return;
	}
	CAkMusicSwitchCtx * pParent = static_cast<CAkMusicSwitchCtx *>( m_pParentCtx );
	if( pParent && pParent->HasOrAscendentHasPendingTransition() )
	{
		AKDIAG( L"akdiag: music switch 0x%08x: change to 0x%08x delayed (a transition is pending above)", m_pSwitchCntrNode->key, in_Switch );
		m_delayedSwitchID = in_Switch;
		m_targetSwitchID = in_Switch;
		return;
	}
	PerformSwitchChange( in_Switch );
	TryPropagateDelayedSwitchChange();
	m_targetSwitchID = in_Switch;
}

/** 2012 0x96c140, 2013 0x968960. */
void CAkMusicSwitchCtx::PerformDelayedSwitchChange()
{
	if( m_delayedSwitchID )
	{
		PerformSwitchChange( m_delayedSwitchID );
	}
	TryPropagateDelayedSwitchChange();
}
