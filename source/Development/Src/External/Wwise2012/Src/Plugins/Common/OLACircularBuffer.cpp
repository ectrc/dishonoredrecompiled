/*=============================================================================
	OLACircularBuffer.cpp - DSP::CAkOLACircularBuffer, Wwise 2012.1 (plugins\common\olacircularbuffer.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes.
=============================================================================*/
#include "OLACircularBuffer.h"
#include <string.h>

namespace DSP
{

/** 2012 0x993070, 2013 0x990360. */
AKRESULT CAkOLACircularBuffer::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uSize, AkUInt32 in_uWindowSize )
{
	m_uWindowSize = in_uWindowSize;
	return CAkCircularBuffer::Init( in_pAllocator, in_uSize );
}

/** 2012 0x993030, 2013 0x990320: the overlap tail (one window past the write head) has been read out. */
bool CAkOLACircularBuffer::IsDoneTail()
{
	if( m_uFramesReady )
	{
		return false;
	}
	const AkUInt32 uTailEnd = ( m_uWriteOffset + m_uWindowSize ) % m_uSize;
	if( uTailEnd < m_uReadOffset )
	{
		return m_uSize + m_uReadOffset - uTailEnd == 0;
	}
	return uTailEnd == m_uReadOffset;
}

/** 2012 0x993090, 2013 0x990380: overlap-add one window at the write head, then advance by the hop. */
bool CAkOLACircularBuffer::PushOverlappedWindow( AkReal32 * in_pfBuffer, AkUInt32 in_uHopSize, AkReal32 * io_pfData )
{
	if( !in_pfBuffer || m_uWindowSize > m_uSize - m_uFramesReady )
	{
		return false;
	}
	AkUInt32 uFramesBeforeWrap = m_uSize - m_uWriteOffset;
	if( m_uWindowSize < uFramesBeforeWrap )
	{
		uFramesBeforeWrap = m_uWindowSize;
	}
	AkReal32 * pfOut = &io_pfData[ m_uWriteOffset ];
	const AkReal32 * pfIn = in_pfBuffer;
	for( AkUInt32 i = 0; i < uFramesBeforeWrap; ++i )
	{
		pfOut[ i ] = pfOut[ i ] + pfIn[ i ];
	}
	const AkUInt32 uFramesAfterWrap = m_uWindowSize - uFramesBeforeWrap;
	pfIn = &in_pfBuffer[ uFramesBeforeWrap ];
	for( AkUInt32 i = 0; i < uFramesAfterWrap; ++i )
	{
		io_pfData[ i ] = pfIn[ i ] + io_pfData[ i ];
	}
	m_uFramesReady += in_uHopSize;
	m_uWriteOffset = ( in_uHopSize + m_uWriteOffset ) % m_uSize;
	return true;
}

/** 2012 0x993210, 2013 0x990500: pops (and clears) frames; with no more input data the overlap tail drains. */
AkUInt32 CAkOLACircularBuffer::PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, bool in_bNoMoredata, AkReal32 * io_pfData )
{
	if( !out_pfBuffer || !in_RequestedFrames )
	{
		return 0;
	}
	AkUInt32 uFramesToPop = in_RequestedFrames;
	if( !in_bNoMoredata || m_uFramesReady )
	{
		if( m_uFramesReady < in_RequestedFrames )
		{
			uFramesToPop = m_uFramesReady;
		}
		m_uFramesReady -= uFramesToPop;
	}
	else
	{
		const AkUInt32 uTailEnd = ( m_uWriteOffset + m_uWindowSize ) % m_uSize;
		const AkUInt32 uTailFrames = uTailEnd <= m_uReadOffset ? m_uSize + m_uReadOffset - uTailEnd : uTailEnd - m_uReadOffset;
		if( uTailFrames < in_RequestedFrames )
		{
			uFramesToPop = uTailFrames;
		}
	}
	AkUInt32 uFramesBeforeWrap = uFramesToPop;
	if( uFramesToPop >= m_uSize - m_uReadOffset )
	{
		uFramesBeforeWrap = m_uSize - m_uReadOffset;
	}
	memcpy( out_pfBuffer, &io_pfData[ m_uReadOffset ], sizeof( AkReal32 ) * uFramesBeforeWrap );
	memset( &io_pfData[ m_uReadOffset ], 0, sizeof( AkReal32 ) * uFramesBeforeWrap );
	if( uFramesToPop != uFramesBeforeWrap )
	{
		memcpy( &out_pfBuffer[ uFramesBeforeWrap ], io_pfData, sizeof( AkReal32 ) * ( uFramesToPop - uFramesBeforeWrap ) );
		memset( io_pfData, 0, sizeof( AkReal32 ) * ( uFramesToPop - uFramesBeforeWrap ) );
	}
	m_uReadOffset = ( uFramesToPop + m_uReadOffset ) % m_uSize;
	return uFramesToPop;
}

/** 2012 0x993300, 2013 0x9905f0. */
bool CAkOLACircularBuffer::PushOverlappedWindow( AkReal32 * in_pfBuffer, AkUInt32 in_uHopSize )
{
	return PushOverlappedWindow( in_pfBuffer, in_uHopSize, m_pfData );
}

/** 2012 0x993320, 2013 0x990610. */
AkUInt32 CAkOLACircularBuffer::PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, bool in_bNoMoredata )
{
	return PopFrames( out_pfBuffer, in_RequestedFrames, in_bNoMoredata, m_pfData );
}

}
