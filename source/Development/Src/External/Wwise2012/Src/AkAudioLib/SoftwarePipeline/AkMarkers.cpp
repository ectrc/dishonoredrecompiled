/*=============================================================================
	AkMarkers.cpp - CAkMarkers (akmarkers.cpp, 2012 0x960300..0x9605b0; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26).
=============================================================================*/
#include "AkMarkers.h"
#include "AkPBI.h"
#include "AkPlayingMgr.h"
#include <stdlib.h>

/** 2012 0x960300. */
CAkMarkers::CAkMarkers()
	: m_pMarkers( NULL )
{
	m_hdrMarkers.uNumMarkers = 0;
}

/** 2012 0x960310. */
AKRESULT CAkMarkers::Allocate( AkUInt32 in_uNumMarkers )
{
	m_hdrMarkers.uNumMarkers = in_uNumMarkers;
	m_pMarkers = (AkAudioMarker *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( AkAudioMarker ) * in_uNumMarkers );
	if( m_pMarkers )
	{
		return AK_Success;
	}
	m_hdrMarkers.uNumMarkers = 0;
	return AK_InsufficientMemory;
}

/** 2012 0x960350. */
void CAkMarkers::Free()
{
	if( m_pMarkers )
	{
		for( AkUInt32 i = 0; i < m_hdrMarkers.uNumMarkers; ++i )
		{
			if( m_pMarkers[ i ].strLabel )
			{
				AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pMarkers[ i ].strLabel );
				m_pMarkers[ i ].strLabel = NULL;
			}
		}
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pMarkers );
		m_pMarkers = NULL;
	}
	m_hdrMarkers.uNumMarkers = 0;
}

/** 2012 0x9603c0. */
AKRESULT CAkMarkers::SetLabel( AkUInt32 in_idx, char * in_psLabel, AkUInt32 in_uStrSize )
{
	char * pLabel = (char *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, in_uStrSize + 1 );
	if( !pLabel )
	{
		return AK_InsufficientMemory;
	}
	memcpy( pLabel, in_psLabel, in_uStrSize );
	pLabel[ in_uStrSize ] = 0;
	m_pMarkers[ in_idx ].strLabel = pLabel;
	return AK_Success;
}

/** 2012 0x960420: the markers that fall in [start, start + valid frames) of the buffer, if the PBI asked for markers. */
void CAkMarkers::CopyRelevantMarkers( CAkPBI * in_pCtx, AkPipelineBuffer & io_buffer, AkUInt32 in_ulBufferStartPos )
{
	if( !m_pMarkers || !( in_pCtx->m_uRegisteredNotif & AK_Marker ) )
	{
		return;
	}
	const AkUInt32 uNumFrames = io_buffer.uValidFrames;
	io_buffer.pMarkers = NULL;
	io_buffer.uNumMarkers = 0;
	for( AkUInt32 i = 0; i < m_hdrMarkers.uNumMarkers; ++i )
	{
		const AkUInt32 dwPosition = m_pMarkers[ i ].dwPosition;
		if( dwPosition >= in_ulBufferStartPos && dwPosition < in_ulBufferStartPos + uNumFrames )
		{
			++io_buffer.uNumMarkers;
		}
	}
	if( !io_buffer.uNumMarkers )
	{
		return;
	}
	io_buffer.pMarkers = (AkBufferMarker *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( AkBufferMarker ) * io_buffer.uNumMarkers );
	if( !io_buffer.pMarkers )
	{
		io_buffer.uNumMarkers = 0;
		return;
	}
	AkBufferMarker * pBufferMarker = io_buffer.pMarkers;
	for( AkUInt32 i = 0; i < m_hdrMarkers.uNumMarkers; ++i )
	{
		const AkUInt32 dwPosition = m_pMarkers[ i ].dwPosition;
		if( dwPosition >= in_ulBufferStartPos && dwPosition < in_ulBufferStartPos + uNumFrames )
		{
			pBufferMarker->pContext = in_pCtx;
			pBufferMarker->dwPositionInBuffer = dwPosition - in_ulBufferStartPos;
			pBufferMarker->marker = m_pMarkers[ i ];
			++pBufferMarker;
		}
	}
}

/** 2012 0x960530. */
void CAkMarkers::NotifyRelevantMarkers( CAkPBI * in_pCtx, AkUInt32 in_uStartSample, AkUInt32 in_uStopSample )
{
	if( !m_pMarkers || !( in_pCtx->m_uRegisteredNotif & AK_Marker ) )
	{
		return;
	}
	for( AkUInt32 i = 0; i < m_hdrMarkers.uNumMarkers; ++i )
	{
		if( m_pMarkers[ i ].dwPosition >= in_uStartSample && m_pMarkers[ i ].dwPosition < in_uStopSample )
		{
			g_pPlayingMgr->NotifyMarker( in_pCtx, &m_pMarkers[ i ] );
		}
	}
}

/** 2012 0x960590. */
void CAkMarkers::TimeSkipMarkers( CAkPBI * in_pCtx, AkUInt32 in_ulCurrSampleOffset, AkUInt32 in_uSkippedSamples, AkUInt32 in_uFileEnd )
{
	NotifyRelevantMarkers( in_pCtx, in_ulCurrSampleOffset, in_ulCurrSampleOffset + in_uSkippedSamples );
}

/** 2012 0x9605b0. */
const AkAudioMarker * CAkMarkers::GetClosestMarker( AkUInt32 in_uPosition ) const
{
	const AkAudioMarker * pClosest = NULL;
	AkUInt32 uSmallestDistance = 0;
	for( AkUInt32 i = 0; i < m_hdrMarkers.uNumMarkers; ++i )
	{
		const AkUInt32 uDistance = (AkUInt32)abs( (AkInt32)( m_pMarkers[ i ].dwPosition - in_uPosition ) );
		if( !pClosest || uDistance < uSmallestDistance )
		{
			pClosest = &m_pMarkers[ i ];
			uSmallestDistance = uDistance;
		}
	}
	return pClosest;
}
