/*=============================================================================
	AkSequencableSegmentCtx.cpp - CAkSequencableSegmentCtx (aksequencablesegmentctx.cpp, 2012 0x969e10..0x96a200).

	DISHONORED(port): agent IH4 (wave 26).
=============================================================================*/
#include "AkSequencableSegmentCtx.h"
#include "AkMusicSegment.h"
#include "AkSegmentCtx.h"
#include "AkMatrixSequencer.h"
#include "AkScheduleWindow.h"

/** 2012 0x969e10, 2013 0x966630. */
CAkSequencableSegmentCtx::CAkSequencableSegmentCtx( CAkMusicSegment * in_pSegmentNode, CAkMusicCtx * in_pParentCtx )
	: CAkChainCtx( in_pParentCtx )
	, m_pSegmentNode( in_pSegmentNode )
{
	if( m_pSegmentNode )
	{
		m_pSegmentNode->AddRef();
	}
}

/** 2012 0x969f40 (the deleting destructor's body). */
CAkSequencableSegmentCtx::~CAkSequencableSegmentCtx()
{
	if( m_pSegmentNode )
	{
		m_pSegmentNode->Release();
	}
}

/** 2012 0x969f40 slot 15 (FDefaultMaterialInstance::IsSelected folded): the segment node. */
CAkMusicNode * CAkSequencableSegmentCtx::Node()
{
	return m_pSegmentNode;
}

/**
 * 2012 0x969e50: the segment item (stopped after its post-exit) and a silent item at its exit cue; the context is
 * prepared from in_iStartPos. AK_Fail if the start is past the segment's end.
 */
AKRESULT CAkSequencableSegmentCtx::SetupSegmentChain( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams, AkInt32 in_iStartPos, AkInt32 & out_iLookAheadDuration )
{
	out_iLookAheadDuration = 0;
	if( in_iStartPos >= (AkInt32)( m_pSegmentNode->ActiveDuration() + m_pSegmentNode->PostExitDuration() ) )
	{
		return AK_Fail;
	}
	CAkSegmentCtx * pSegmentCtx = m_pSegmentNode->CreateLowLevelSegmentCtxAndAddRef( this, in_GameObject, in_rUserparams );
	if( !pSegmentCtx )
	{
		return AK_Fail;
	}
	CAkScheduledItem * pItem = EnqueueItem( 0, pSegmentCtx );
	pSegmentCtx->Release();
	if( !pItem )
	{
		return AK_Fail;
	}
	m_iLocalTime = 0;
	AkUInt32 uSelectedCue;
	out_iLookAheadDuration = Prepare( NULL, in_iStartPos, uSelectedCue, 0 );
	pItem->AttachStopCmd( 0, AkCurveInterpolation_Linear, (AkInt32)( m_pSegmentNode->ActiveDuration() + m_pSegmentNode->PostExitDuration() ) );
	return EnqueueItem( m_pSegmentNode->ActiveDuration(), NULL ) ? AK_Success : AK_Fail;
}

/** 2012 0x969f90, 2013 0x9667b0: prepared from the start of the pre-entry; the context starts its look-ahead early. */
AKRESULT CAkSequencableSegmentCtx::Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	AKRESULT eResult = CAkMatrixAwareCtx::Init( in_GameObject, in_rUserparams );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	AkInt32 iLookAheadDuration;
	eResult = SetupSegmentChain( in_GameObject, in_rUserparams, -m_pSegmentNode->PreEntryDuration(), iLookAheadDuration );
	m_iLocalTime = iLookAheadDuration;
	return eResult;
}

/** 2012 0x969fe0, 2013 0x966800: the children are cancelled and the chain rebuilt at the position, re-anchored on the sequencer's time. */
AKRESULT CAkSequencableSegmentCtx::Seek( AkInt32 in_iSeekPosition )
{
	if( m_eStatus > CtxStatusPlaying )
	{
		return AK_Fail;
	}
	AddRef();
	CAkChildCtx * pChild = m_listChildren.m_pFirst;
	while( pChild )
	{
		CAkMusicCtx * pChildCtx = static_cast<CAkMusicCtx *>( pChild );
		pChild = pChild->pNextLightItem;
		pChildCtx->_Cancel();
	}
	Flush();
	AkInt32 iLookAhead;
	const AKRESULT eResult = SetupSegmentChain( m_pSequencer->m_pGameObj, m_pSequencer->m_UserParams, in_iSeekPosition, iLookAhead );
	if( eResult == AK_Success )
	{
		AkInt64 iLocalTime = m_iLocalTime;
		if( m_pParentCtx )
		{
			iLocalTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
		}
		m_iLocalTime = (AkInt64)m_pSequencer->m_uTime + iLookAhead - iLocalTime;
		Release();
		return AK_Success;
	}
	TransParams transParams;
	transParams.TransitionTime = 0;
	transParams.eFadeCurve = AkCurveInterpolation_Linear;
	_Stop( transParams, 0 );
	Release();
	return eResult;
}

/** 2012 0x96a0c0, 2013 0x9668e0: as the matrix-aware context's, on the very first item. */
AKRESULT CAkSequencableSegmentCtx::GetPlayingSegmentInfo( AkSegmentInfo & out_segmentInfo )
{
	CAkScheduleWindow window( this, true );
	if( !window.m_itScheduledItem.pItem )
	{
		return AK_Fail;
	}
	AkInt64 iLocalTime = m_iLocalTime;
	const AkInt64 iTime = (AkInt64)m_pSequencer->m_uTime;
	if( m_pParentCtx )
	{
		iLocalTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
	}
	AkInt32 iSegmentPosition = 0;
	if( window.m_itScheduledItem.pItem->m_pSegment )
	{
		iSegmentPosition = (AkInt32)window.m_itScheduledItem.CtxTimeToSegmentPosition( iTime - iLocalTime - window.ChainCtxTimeRelativeToLevel( 0 ) );
	}
	return window.m_itScheduledItem.pItem->GetInfo( iSegmentPosition, out_segmentInfo );
}

/** 2012 0x96a190, 2013 0x9669b0: ms to samples (frequency / 1000 truncated), not before the pre-entry, optionally snapped to a cue. */
AKRESULT CAkSequencableSegmentCtx::SeekTimeAbsolute( AkTimeMs & io_position, bool in_bSnapToCue )
{
	AkInt32 iSeekPosition = io_position * (AkInt32)( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	if( iSeekPosition < -m_pSegmentNode->PreEntryDuration() )
	{
		iSeekPosition = -m_pSegmentNode->PreEntryDuration();
	}
	if( in_bSnapToCue )
	{
		iSeekPosition = m_pSegmentNode->GetClosestCuePosition( iSeekPosition );
		io_position = AkTimeConv::SamplesToMilliseconds( iSeekPosition );
	}
	return Seek( iSeekPosition );
}

/** 2012 0x96a200, 2013 0x966a20: a fraction of the active duration. */
AKRESULT CAkSequencableSegmentCtx::SeekPercent( AkReal32 & io_fPercent, bool in_bSnapToCue )
{
	AkInt32 iSeekPosition = (AkInt32)( (AkReal64)m_pSegmentNode->ActiveDuration() * io_fPercent );
	if( in_bSnapToCue )
	{
		iSeekPosition = m_pSegmentNode->GetClosestCuePosition( iSeekPosition );
		io_fPercent = (AkReal32)( (AkReal32)iSeekPosition / (AkReal64)m_pSegmentNode->ActiveDuration() );
	}
	return Seek( iSeekPosition );
}
