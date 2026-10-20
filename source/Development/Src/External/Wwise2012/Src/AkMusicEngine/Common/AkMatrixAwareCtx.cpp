/*=============================================================================
	AkMatrixAwareCtx.cpp - CAkMatrixAwareCtx (akmatrixawarectx.cpp, 2012 0x964780..0x9649a0).

	DISHONORED(port): agent IH4 (wave 26).
=============================================================================*/
#include "AkMatrixAwareCtx.h"
#include "AkMatrixSequencer.h"
#include "AkMusicRenderer.h"
#include "AkScheduleWindow.h"
#include "AkSegmentChain.h"
#include "AkSegmentCtx.h"

/** 2012 0x9647a0, 2013 0x960fe0. */
CAkMatrixAwareCtx::CAkMatrixAwareCtx( CAkMusicCtx * in_parent )
	: CAkMusicCtx( in_parent )
	, m_pSequencer( NULL )
	, m_iLocalTime( 0 )
	, pNextTopLevelSibling( NULL )
{
}

/** 2012 0x9647e0, 2013 0x961020. */
CAkMatrixAwareCtx::~CAkMatrixAwareCtx()
{
}

/** 2012 0x964800, 2013 0x961040: how long the context plays silently before its first audible sample (the streaming look-ahead). */
AkUInt32 CAkMatrixAwareCtx::GetSilentDuration()
{
	AkInt64 iPlayTime;
	AkInt64 iPlayTimeAudible;
	QueryLookAheadInfo( iPlayTime, iPlayTimeAudible );
	return (AkUInt32)( iPlayTimeAudible - iPlayTime );
}

/** 2012 0x964830, 2013 0x961070: a top-level context's sequencer drops its stingers. */
void CAkMatrixAwareCtx::OnStopped()
{
	CAkMusicCtx::OnStopped();
	if( !m_pParentCtx && m_pSequencer )
	{
		m_pSequencer->OnStopped();
	}
}

/** 2012 0x964850, 2013 0x961090: the segment of the first scheduled item (and its parent node). */
CAkMusicSegment * CAkMatrixAwareCtx::GetFirstSegmentNode( CAkMusicNode ** out_ppParentNode )
{
	CAkScheduleWindow wndDest( this, true );
	if( wndDest.m_itScheduledItem.pItem )
	{
		return wndDest.GetNode( out_ppParentNode );
	}
	if( out_ppParentNode )
	{
		*out_ppParentNode = NULL;
	}
	return NULL;
}

/** 2012 0x9648c0, 2013 0x961100. */
void CAkMatrixAwareCtx::SetSequencer( CAkMatrixSequencer * in_pSequencer )
{
	m_pSequencer = in_pSequencer;
}

/** 2012 0x964920, 2013 0x961160: a top-level context registers with the renderer (which makes its sequencer); a child inherits it. */
AKRESULT CAkMatrixAwareCtx::Init( CAkRegisteredObj * in_pGameObj, UserParams & in_rUserparams )
{
	CAkMusicCtx::Init( in_pGameObj, in_rUserparams );
	if( !m_pParentCtx )
	{
		return CAkMusicRenderer::m_pMusicRenderer->AddChild( this, in_rUserparams, in_pGameObj );
	}
	m_pSequencer = static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->m_pSequencer;
	return AK_Success;
}

/** 2012 0x964960, 2013 0x9611a0: the top level runs its sequencer (stingers, music callbacks); a context in its last frame stops. */
void CAkMatrixAwareCtx::ProcessEpilogue( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples )
{
	if( !m_pParentCtx )
	{
		m_pSequencer->Process( in_iCurrentTime, in_uNumSamples );
	}
	if( m_eStatus & CtxStatusLastFrame )
	{
		OnStopped();
	}
	Release();
}

/** 2012 0x9649a0, 2013 0x9611e0: the playing segment's position and durations, as seen at the sequencer's time. */
AKRESULT CAkMatrixAwareCtx::GetPlayingSegmentInfo( AkSegmentInfo & out_segmentInfo )
{
	CAkScheduleWindow window( this, false );
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

/** 2012 0x964780, 2013 0x960fc0: the local time, plus every ancestor's. */
AkInt64 CAkMatrixAwareCtx::GetAbsoluteTimeOffset()
{
	AkInt64 iTime = m_iLocalTime;
	if( m_pParentCtx )
	{
		iTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
	}
	return iTime;
}
