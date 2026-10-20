/*=============================================================================
	CircularBuffer.cpp - DSP::CAkCircularBuffer, Wwise 2012.1 (plugins\common\circularbuffer.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes.
=============================================================================*/
#include "CircularBuffer.h"
#include <string.h>

namespace DSP
{

/** 2012 0x993340, 2013 0x990630. */
AKRESULT CAkCircularBuffer::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uSize )
{
	m_uSize = in_uSize;
	m_uFramesReady = 0;
	m_uReadOffset = 0;
	m_uWriteOffset = 0;
	m_pfData = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * in_uSize );
	return m_pfData ? AK_Success : AK_InsufficientMemory;
}

/** 2012 0x993380, 2013 0x990670. */
void CAkCircularBuffer::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pfData )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pfData );
		m_pfData = NULL;
	}
	m_uFramesReady = 0;
}

/** 2012 0x9933b0, 2013 0x9906a0. */
void CAkCircularBuffer::Reset()
{
	if( m_pfData )
	{
		memset( m_pfData, 0, sizeof( AkReal32 ) * m_uSize );
	}
	m_uFramesReady = 0;
	m_uReadOffset = 0;
	m_uWriteOffset = 0;
}

/** 2012 0x9933f0, 2013 0x9906e0. */
AkUInt32 CAkCircularBuffer::AdvanceFrames( AkUInt32 in_uNumFrames )
{
	const AkUInt32 uFramesToAdvance = in_uNumFrames < m_uFramesReady ? in_uNumFrames : m_uFramesReady;
	m_uFramesReady -= uFramesToAdvance;
	m_uReadOffset = ( uFramesToAdvance + m_uReadOffset ) % m_uSize;
	return uFramesToAdvance;
}

/** 2012 0x993420, 2013 0x990710. */
AkUInt32 CAkCircularBuffer::PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 * io_pfData )
{
	const AkUInt32 uFramesToPush = ( m_uSize - m_uFramesReady ) < in_NumFrames ? ( m_uSize - m_uFramesReady ) : in_NumFrames;
	if( !in_pfBuffer || !uFramesToPush )
	{
		return 0;
	}
	AkUInt32 uFramesBeforeWrap = m_uSize - m_uWriteOffset;
	if( uFramesToPush < uFramesBeforeWrap )
	{
		uFramesBeforeWrap = uFramesToPush;
	}
	if( uFramesBeforeWrap )
	{
		memcpy( &io_pfData[ m_uWriteOffset ], in_pfBuffer, sizeof( AkReal32 ) * uFramesBeforeWrap );
	}
	if( uFramesToPush != uFramesBeforeWrap )
	{
		memcpy( io_pfData, &in_pfBuffer[ uFramesBeforeWrap ], sizeof( AkReal32 ) * ( uFramesToPush - uFramesBeforeWrap ) );
	}
	m_uFramesReady += uFramesToPush;
	m_uWriteOffset = ( uFramesToPush + m_uWriteOffset ) % m_uSize;
	return uFramesToPush;
}

/** 2012 0x9934b0, 2013 0x9907a0. */
AkUInt32 CAkCircularBuffer::PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames, AkReal32 * io_pfData )
{
	const AkUInt32 uFramesToPop = m_uFramesReady < in_RequestedFrames ? m_uFramesReady : in_RequestedFrames;
	if( !out_pfBuffer || !uFramesToPop )
	{
		return 0;
	}
	AkUInt32 uFramesBeforeWrap = m_uSize - m_uReadOffset;
	if( uFramesToPop < uFramesBeforeWrap )
	{
		uFramesBeforeWrap = uFramesToPop;
	}
	memcpy( out_pfBuffer, &io_pfData[ m_uReadOffset ], sizeof( AkReal32 ) * uFramesBeforeWrap );
	if( uFramesToPop != uFramesBeforeWrap )
	{
		memcpy( &out_pfBuffer[ uFramesBeforeWrap ], io_pfData, sizeof( AkReal32 ) * ( uFramesToPop - uFramesBeforeWrap ) );
	}
	m_uFramesReady -= uFramesToPop;
	m_uReadOffset = ( uFramesToPop + m_uReadOffset ) % m_uSize;
	return uFramesToPop;
}

/** 2012 0x993540, 2013 0x990830: copies one block without consuming it; with no more input a short tail is zero padded. */
bool CAkCircularBuffer::ReadFrameBlock( AkReal32 * out_pfBuffer, AkUInt32 in_uBlockSize, bool in_bNoMoreInputData, AkReal32 * io_pfData )
{
	if( !out_pfBuffer || !in_uBlockSize )
	{
		return false;
	}
	if( in_bNoMoreInputData )
	{
		if( !m_uFramesReady )
		{
			return false;
		}
		const AkUInt32 uFramesAvailable = m_uFramesReady >= in_uBlockSize ? in_uBlockSize : m_uFramesReady;
		if( in_uBlockSize != uFramesAvailable )
		{
			memset( &out_pfBuffer[ uFramesAvailable ], 0, sizeof( AkReal32 ) * ( in_uBlockSize - uFramesAvailable ) );
		}
	}
	else if( m_uFramesReady < in_uBlockSize )
	{
		return false;
	}
	AkUInt32 uFramesBeforeWrap = m_uSize - m_uReadOffset;
	if( in_uBlockSize < uFramesBeforeWrap )
	{
		uFramesBeforeWrap = in_uBlockSize;
	}
	memcpy( out_pfBuffer, &io_pfData[ m_uReadOffset ], sizeof( AkReal32 ) * uFramesBeforeWrap );
	if( in_uBlockSize != uFramesBeforeWrap )
	{
		memcpy( &out_pfBuffer[ uFramesBeforeWrap ], io_pfData, sizeof( AkReal32 ) * ( in_uBlockSize - uFramesBeforeWrap ) );
	}
	return true;
}

/** 2012 0x9935e0, 2013 0x9908d0. */
AkUInt32 CAkCircularBuffer::PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames )
{
	return PushFrames( in_pfBuffer, in_NumFrames, m_pfData );
}

/** 2012 0x993600, 2013 0x9908f0. */
AkUInt32 CAkCircularBuffer::PopFrames( AkReal32 * out_pfBuffer, AkUInt32 in_RequestedFrames )
{
	return PopFrames( out_pfBuffer, in_RequestedFrames, m_pfData );
}

/** 2012 0x993620, 2013 0x990910. */
bool CAkCircularBuffer::ReadFrameBlock( AkReal32 * out_pfBuffer, AkUInt32 in_uBlockSize, bool in_bNoMoreInputData )
{
	return ReadFrameBlock( out_pfBuffer, in_uBlockSize, in_bNoMoreInputData, m_pfData );
}

}
