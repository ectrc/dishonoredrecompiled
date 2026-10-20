/*=============================================================================
	AkDelayPitchShift.cpp - AK::DSP::AkDelayPitchShift, Wwise 2012.1 (plugins/common/akdelaypitchshift.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Kept as retail: the second tap is
	interpolated with the first tap's fractional part (both taps advance by the same delta, so their fractions only
	differ by the half-length's own fraction, zero for an even delay length); the crossfade weight is
	|delay1 - half| / half.
=============================================================================*/
#include "AkDelayPitchShift.h"
#include <math.h>

namespace AK
{
namespace DSP
{

/** 2012 0x9966f0, 2013 0x98cb80. */
AkDelayPitchShift::AkDelayPitchShift()
	: m_fReadWriteRateDelta( 0.f )
	, m_uNumChannels( 0 )
	, m_uDelayLength( 0 )
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		m_fFractDelay[ i ] = 0.f;
	}
}

/** 2012 0x996e10, 2013 0x98db20. */
void AkDelayPitchShift::SetPitchFactor( AkReal32 in_fPitchFactor )
{
	if( in_fPitchFactor == 1.f )
	{
		m_fReadWriteRateDelta = 0.f;
		for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
		{
			m_fFractDelay[ i ] = 0.f;
		}
	}
	else
	{
		m_fReadWriteRateDelta = 1.f - in_fPitchFactor;
	}
}

/** 2012 0x996e60, 2013 0x98db70. */
AKRESULT AkDelayPitchShift::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 in_MaxDelayTime, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate )
{
	const double dDelayLength = floor( (double)in_uSampleRate * in_MaxDelayTime * 0.001 );
	m_fReadWriteRateDelta = 0.f;
	m_uNumChannels = in_uNumChannels;
	m_uDelayLength = ( (AkUInt32)(AkInt64)dDelayLength + 3 ) & ~3u;
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_fFractDelay[ i ] = 0.f;
		if( m_uDelayLength )
		{
			const AKRESULT eResult = m_DelayLines[ i ].Init( in_pAllocator, m_uDelayLength, 1 );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x996f30, 2013 0x98dc40. */
void AkDelayPitchShift::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_DelayLines[ i ].Term( in_pAllocator );
	}
}

/** 2012 0x996f60, 2013 0x98dc70. */
void AkDelayPitchShift::Reset()
{
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_DelayLines[ i ].Reset();
	}
}

/** 2012 0x996f90, 2013 0x98dca0. */
void AkDelayPitchShift::ProcessChannel( AkReal32 * in_pfInBuf, AkReal32 * out_pfOutBuf, AkUInt32 in_uNumFrames, AkUInt32 in_uChanIndex )
{
	const int iAllocatedDelayLength = (int)m_uDelayLength;
	const AkReal32 fReadWriteRateDelta = m_fReadWriteRateDelta;
	const AkReal32 fDelayLength = (AkReal32)m_uDelayLength;
	const AkReal32 fHalfDelayLength = (AkReal32)( m_uDelayLength / 2 );
	CAkDelayLineMemory<AkReal32, 1> & rDelay = m_DelayLines[ in_uChanIndex ];
	AkUInt32 uOffset = rDelay.GetCurrentOffset();
	AkReal32 * pfDelay = rDelay.GetChannel( 0 );
	AkReal32 fFractDelay1 = m_fFractDelay[ in_uChanIndex ];
	const AkReal32 fHalfLengthNorm = (AkReal32)( 1.0 / (double)( m_uDelayLength / 2 ) );
	AkUInt32 uFramesRemainingToProcess = in_uNumFrames;
	AkUInt32 uFramesBeforeWrap = m_uDelayLength - uOffset;
	while( uFramesRemainingToProcess )
	{
		const AkUInt32 uFramesToProcess = uFramesRemainingToProcess >= uFramesBeforeWrap ? uFramesBeforeWrap : uFramesRemainingToProcess;
		for( AkUInt32 i = 0; i < uFramesToProcess; ++i )
		{
			fFractDelay1 = fFractDelay1 + fReadWriteRateDelta;
			AkReal32 fFractDelay2 = fFractDelay1 + fHalfDelayLength;
			if( fFractDelay1 >= fDelayLength )
			{
				fFractDelay1 = fFractDelay1 - fDelayLength;
			}
			if( fFractDelay2 >= fDelayLength )
			{
				fFractDelay2 = fFractDelay2 - fDelayLength;
			}
			if( fFractDelay1 < 0.f )
			{
				fFractDelay1 = fFractDelay1 + fDelayLength;
			}
			if( fFractDelay2 < 0.f )
			{
				fFractDelay2 = fFractDelay2 + fDelayLength;
			}
			const AkReal32 fWriteOffset = (AkReal32)uOffset;
			const AkReal32 fReadPos1 = fWriteOffset - fFractDelay1;
			const AkReal32 fReadPos1Floor = (AkReal32)floor( fReadPos1 );
			int iReadPos1 = (int)fReadPos1Floor;
			int iReadPos2 = (int)floor( fWriteOffset - fFractDelay2 );
			if( iReadPos1 >= iAllocatedDelayLength )
			{
				iReadPos1 -= iAllocatedDelayLength;
			}
			if( iReadPos2 >= iAllocatedDelayLength )
			{
				iReadPos2 -= iAllocatedDelayLength;
			}
			if( iReadPos1 < 0 )
			{
				iReadPos1 += iAllocatedDelayLength;
			}
			if( iReadPos2 < 0 )
			{
				iReadPos2 += iAllocatedDelayLength;
			}
			const AkReal32 fFrac = fReadPos1 - fReadPos1Floor;
			int iReadPosNext1 = iReadPos1 + 1;
			if( iReadPosNext1 >= iAllocatedDelayLength )
			{
				iReadPosNext1 = 0;
			}
			int iReadPosNext2 = iReadPos2 + 1;
			if( iReadPosNext2 >= iAllocatedDelayLength )
			{
				iReadPosNext2 = 0;
			}
			const AkReal32 fGain2 = (AkReal32)fabs( fFractDelay1 - fHalfDelayLength ) * fHalfLengthNorm;
			const AkReal32 fTap1 = ( ( pfDelay[ iReadPos1 ] * ( 1.f - fFrac ) ) + ( pfDelay[ iReadPosNext1 ] * fFrac ) ) * ( 1.f - fGain2 );
			const AkReal32 fTap2a = pfDelay[ iReadPos2 ] * ( 1.f - fFrac );
			const AkReal32 fTap2b = pfDelay[ iReadPosNext2 ];
			pfDelay[ uOffset ] = *in_pfInBuf++;
			*out_pfOutBuf++ = fTap1 + ( ( fTap2a + fTap2b * fFrac ) * fGain2 );
			++uOffset;
		}
		if( uOffset == m_uDelayLength )
		{
			uOffset = 0;
		}
		uFramesRemainingToProcess -= uFramesToProcess;
		uFramesBeforeWrap = m_uDelayLength - uOffset;
	}
	rDelay.SetCurrentOffset( uOffset );
	m_fFractDelay[ in_uChanIndex ] = fFractDelay1;
}

}
}
