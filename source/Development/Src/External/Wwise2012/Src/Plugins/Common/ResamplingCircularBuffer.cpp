/*=============================================================================
	ResamplingCircularBuffer.cpp - DSP::CAkResamplingCircularBuffer, Wwise 2012.1 (plugins/common/resamplingcircularbuffer.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile. Only the harmonizer uses it; the 2013 exe no longer links it (no 2013 twin).
=============================================================================*/
#include "ResamplingCircularBuffer.h"
#include <string.h>

namespace DSP
{

/** 2012 0x9948a0 (no 2013 twin). */
void CAkResamplingCircularBuffer::Reset()
{
	CAkCircularBuffer::Reset();
	m_fPastVal = 0.f;
	m_fInterpLoc = 0.f;
}

/**
 * 2012 0x9948c0 (no 2013 twin): linear-interpolation resampler into the ring; m_fInterpLoc carries the read position
 * across calls (negative = between the previous call's last sample m_fPastVal and this call's first). Returns the
 * input frames consumed.
 */
AkUInt32 CAkResamplingCircularBuffer::PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 * io_pfData, AkReal32 in_fResamplingFactor )
{
	const AkUInt32 uOutBufferFrames = m_uSize - m_uFramesReady;
	AkUInt32 uWriteOffset = m_uWriteOffset;
	if( m_uSize == m_uFramesReady )
	{
		return 0;
	}
	const AkUInt16 uNumFrames = (AkUInt16)in_NumFrames;
	AkReal32 fInterpLoc = m_fInterpLoc;
	AkUInt32 uFramesConsumed = 0;
	AkUInt32 uFramesProduced = 0;
	AkReal32 fPrev, fNext;
	AkReal32 fConsumed;
	while( 1 )
	{
		if( fInterpLoc >= 0.f )
		{
			const int iIndex = (int)fInterpLoc;
			if( iIndex >= uNumFrames - 1 )
			{
				uFramesConsumed = uNumFrames;
				if( iIndex <= uNumFrames - 1 )
				{
					m_fPastVal = in_pfBuffer[ iIndex ];
					fConsumed = (AkReal32)( iIndex + 1 );
				}
				else
				{
					fConsumed = (AkReal32)uNumFrames;
				}
				fInterpLoc = fInterpLoc - fConsumed;
				goto done;
			}
			fPrev = in_pfBuffer[ iIndex ];
			fNext = in_pfBuffer[ iIndex + 1 ];
			uFramesConsumed = 0;
		}
		else
		{
			fPrev = m_fPastVal;
			fNext = in_pfBuffer[ 0 ];
		}
		if( uFramesProduced == uOutBufferFrames )
		{
			break;
		}
		const AkReal32 fFrac = fInterpLoc < 0.f ? fInterpLoc + 1.f : fInterpLoc - (AkReal32)(AkUInt32)(AkInt64)fInterpLoc;
		io_pfData[ uWriteOffset++ ] = ( fFrac * ( fNext - fPrev ) ) + fPrev;
		++uFramesProduced;
		fInterpLoc = fInterpLoc + in_fResamplingFactor;
		if( uWriteOffset == m_uSize )
		{
			uWriteOffset = 0;
		}
	}
	m_fPastVal = fPrev;
	if( fInterpLoc > 0.f )
	{
		const int iIndex = (int)fInterpLoc;
		uFramesConsumed = iIndex + 1;
		fInterpLoc = fInterpLoc - (AkReal32)( iIndex + 1 );
	}
done:
	m_uWriteOffset = uWriteOffset;
	m_fInterpLoc = fInterpLoc;
	m_uFramesReady += uFramesProduced;
	return uFramesConsumed;
}

/** 2012 0x994a50 (no 2013 twin). */
AkUInt32 CAkResamplingCircularBuffer::PushFrames( AkReal32 * in_pfBuffer, AkUInt32 in_NumFrames, AkReal32 in_fResamplingFactor )
{
	return PushFrames( in_pfBuffer, in_NumFrames, m_pfData, in_fResamplingFactor );
}

}
