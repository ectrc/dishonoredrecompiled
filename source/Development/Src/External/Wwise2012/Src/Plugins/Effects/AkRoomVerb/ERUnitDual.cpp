/*=============================================================================
	ERUnitDual.cpp - DSP::ERUnitDual (2012 0x98bcb0, 0x98ddd0..0x98e110), FDNLPFilter::ComputeFDNLPCoefs (0x98e4c0),
	DSP::FDN4 (0x98e570..0x98ec40); 2013 matched by bytes. Agent IH4 fork F1 (wave 26); deviation named in ERUnitDual.h.
=============================================================================*/
#include "ERUnitDual.h"
#include <math.h>
#include <string.h>

/** 2012 0x98e4c0: g = 10^(-3 L / (T fs)) per pass; the pole b = ln(g) (1 - r^2) / 4 (capped at 0.999) damps the
    highs by the HF decay ratio r; y = g (1 - b) x + b y1 (stored as fA1 = -b). */
void FDNLPFilter::ComputeFDNLPCoefs( AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate, AkUInt32 in_uFDNDelayLength )
{
	const double dGain = pow( 10.0, (double)in_uFDNDelayLength * -3.0 / ( in_fDecayTime * (double)in_uSampleRate ) );
	const double dPole = log10( dGain ) * log( 10.0 ) * ( 1.0 - in_fHFDecayRatio * in_fHFDecayRatio ) * 0.25;
	if( dPole <= 0.999 )
	{
		fB0 = (AkReal32)( dGain * ( 1.0 - dPole ) );
		fA1 = (AkReal32)-dPole;
	}
	else
	{
		fB0 = (AkReal32)( dGain * ( 1.0 - 0.999 ) );
		fA1 = -0.999f;
	}
}

namespace DSP
{
	/** 2012 0x98bcb0. */
	ERUnitDual::ERUnitDual()
		: uDelayLineLength( 0 ), pfDelay( NULL ), uWriteOffset( 0 ), uIndexToNextWrappingTapL( 0 ), uIndexToNextWrappingTapR( 0 ), uNumTapsL( 0 ), uNumTapsR( 0 )
	{
		memset( fTapGainsL, 0, sizeof( fTapGainsL ) );
		memset( fTapGainsR, 0, sizeof( fTapGainsR ) );
		memset( uTapOffsetsL, 0, sizeof( uTapOffsetsL ) );
		memset( uTapOffsetsR, 0, sizeof( uTapOffsetsR ) );
	}

	/** 2012 0x98ddd0: the line spans the earliest to the latest tap (room size scales the pattern by 2^(size/100));
	    delays are multiples of 4 frames, a tap landing on the previous tap's delay is dropped. A tap's offset is its
	    read position relative to write position 0. */
	AKRESULT ERUnitDual::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 in_fRoomSizeScale, const AkRoomVerb::TapInfo * in_pTapInfoLeft, const AkRoomVerb::TapInfo * in_pTapInfoRight,
		AkUInt32 in_uNumTapsL, AkUInt32 in_uNumTapsR, AkUInt32 in_uSampleRate )
	{
		if( !in_uNumTapsL || in_uNumTapsL > 64 || !in_uNumTapsR || in_uNumTapsR > 64 )
		{
			return AK_Fail;
		}
		const double dScale = pow( 2.0, in_fRoomSizeScale * 0.0099999998f );
		const AkReal32 fScale = (AkReal32)dScale;
		const AkReal32 fMinTime = ( in_pTapInfoRight->fTapTime <= in_pTapInfoLeft->fTapTime ? in_pTapInfoRight->fTapTime : in_pTapInfoLeft->fTapTime ) * fScale;
		const AkReal32 fMaxTap = in_pTapInfoLeft[ in_uNumTapsL - 1 ].fTapTime <= in_pTapInfoRight[ in_uNumTapsR - 1 ].fTapTime ? in_pTapInfoRight[ in_uNumTapsR - 1 ].fTapTime : in_pTapInfoLeft[ in_uNumTapsL - 1 ].fTapTime;
		const double dSampleRate = (double)in_uSampleRate;
		AkUInt32 uLength = (AkUInt32)(AkInt64)( dSampleRate * ( dScale * fMaxTap - fMinTime ) * 0.001 );
		if( uLength < 4 )
		{
			uLength = 4;
		}
		else if( uLength > 0xFFFF )
		{
			return AK_Fail;
		}
		uDelayLineLength = uLength & 0xFFFC;
		pfDelay = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * uDelayLineLength );
		if( !pfDelay )
		{
			return AK_InsufficientMemory;
		}
		const AkRoomVerb::TapInfo * aTapInfo[ 2 ] = { in_pTapInfoLeft, in_pTapInfoRight };
		const AkUInt32 aNumTaps[ 2 ] = { in_uNumTapsL, in_uNumTapsR };
		AkUInt16 * aOffsets[ 2 ] = { uTapOffsetsL, uTapOffsetsR };
		AkReal32 * aGains[ 2 ] = { fTapGainsL, fTapGainsR };
		AkUInt16 * aNextWrap[ 2 ] = { &uIndexToNextWrappingTapL, &uIndexToNextWrappingTapR };
		AkUInt16 * aNumKept[ 2 ] = { &uNumTapsL, &uNumTapsR };
		for( AkUInt32 uSide = 0; uSide < 2; ++uSide )
		{
			AkUInt16 uKept = 0;
			AkUInt16 uMaxOffset = 0;
			AkUInt32 uPrevDelay = (AkUInt32)-1;
			*aNextWrap[ uSide ] = 0;
			for( AkUInt32 i = 0; i < aNumTaps[ uSide ]; ++i )
			{
				const AkReal32 fTime = fScale * aTapInfo[ uSide ][ i ].fTapTime;
				AkUInt32 uDelay = (AkUInt32)(AkInt64)( ( fTime - fMinTime ) * dSampleRate * 0.001 ) & 0xFFFFFFFC;
				if( uDelay >= uDelayLineLength )
				{
					uDelay = uDelayLineLength - 4;
				}
				if( uDelay != uPrevDelay )
				{
					aOffsets[ uSide ][ uKept ] = uDelay ? (AkUInt16)( uDelayLineLength - uDelay ) : 0;
					aGains[ uSide ][ uKept ] = aTapInfo[ uSide ][ i ].fTapGain;
					if( aOffsets[ uSide ][ uKept ] > uMaxOffset )
					{
						uMaxOffset = aOffsets[ uSide ][ uKept ];
						*aNextWrap[ uSide ] = uKept;
					}
					++uKept;
				}
				uPrevDelay = uDelay;
			}
			*aNumKept[ uSide ] = uKept;
		}
		uWriteOffset = 0;
		return AK_Success;
	}

	/** 2012 0x98e0c0. */
	void ERUnitDual::Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		if( pfDelay )
		{
			AK_PLUGIN_FREE( in_pAllocator, pfDelay );
			pfDelay = NULL;
		}
		uDelayLineLength = 0;
	}

	/** 2012 0x98e0f0. */
	void ERUnitDual::Reset()
	{
		if( pfDelay )
		{
			memset( pfDelay, 0, sizeof( AkReal32 ) * uDelayLineLength );
		}
	}

	/** 2012 0x98e110: write the input, then sum every tap (left and right, in tap order); each read position advances
	    with the write position and wraps at the end of the line. */
	void ERUnitDual::ProcessBuffer( AkReal32 * in_pfInput, AkReal32 * out_pfEROutputL, AkReal32 * out_pfEROutputR, AkUInt32 in_uNumFrames )
	{
		for( AkUInt32 uFrame = 0; uFrame < in_uNumFrames; ++uFrame )
		{
			pfDelay[ uWriteOffset ] = in_pfInput[ uFrame ];
			AkReal32 fL = 0.f;
			for( AkUInt32 i = 0; i < uNumTapsL; ++i )
			{
				fL = pfDelay[ uTapOffsetsL[ i ] ] * fTapGainsL[ i ] + fL;
				if( ++uTapOffsetsL[ i ] == uDelayLineLength )
				{
					uTapOffsetsL[ i ] = 0;
				}
			}
			AkReal32 fR = 0.f;
			for( AkUInt32 i = 0; i < uNumTapsR; ++i )
			{
				fR = pfDelay[ uTapOffsetsR[ i ] ] * fTapGainsR[ i ] + fR;
				if( ++uTapOffsetsR[ i ] == uDelayLineLength )
				{
					uTapOffsetsR[ i ] = 0;
				}
			}
			out_pfEROutputL[ uFrame ] = fL;
			out_pfEROutputR[ uFrame ] = fR;
			if( ++uWriteOffset == uDelayLineLength )
			{
				uWriteOffset = 0;
			}
		}
	}

	/** 2012 0x98e570: the HF ratio the network holds without gain above 1 at any frequency. */
	AkReal32 FDN4::ComputeMaxStableHFRatio( AkUInt32 in_uDelayLineLength, AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate )
	{
		const AkReal32 fScale = 1.f - in_fHFDecayRatio * in_fHFDecayRatio;
		const double dLog = log10( pow( 10.0, (double)in_uDelayLineLength * -3.0 / ( in_fDecayTime * (double)in_uSampleRate ) ) ) * log( 10.0 ) * 0.25;
		if( dLog * fScale <= 1.0 )
		{
			return (AkReal32)sqrt( 1.0 - fScale );
		}
		return (AkReal32)sqrt( 1.0 - 1.0 / dLog );
	}

	/** 2012 0x98e600: the damping of the four lines from the longest line's stable ratio. */
	void FDN4::ChangeDecay( AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate )
	{
		const AkReal32 fMaxHFRatio = ComputeMaxStableHFRatio( FDNDelayLine[ 3 ].uDelayLineLength, in_fDecayTime, in_fHFDecayRatio, in_uSampleRate );
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			delayLowPassFilter[ i ].ComputeFDNLPCoefs( in_fDecayTime, fMaxHFRatio, in_uSampleRate, FDNDelayLine[ i ].uDelayLineLength );
		}
	}

	/** 2012 0x98e6b0. */
	AKRESULT FDN4::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 * in_uDelayLineLength, AkReal32 in_fDecayTime, AkReal32 in_fHFDecayRatio, AkUInt32 in_uSampleRate )
	{
		const AkReal32 fMaxHFRatio = ComputeMaxStableHFRatio( in_uDelayLineLength[ 3 ], in_fDecayTime, in_fHFDecayRatio, in_uSampleRate );
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			const AKRESULT eResult = FDNDelayLine[ i ].Init( in_pAllocator, in_uDelayLineLength[ i ] );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			delayLowPassFilter[ i ].ComputeFDNLPCoefs( in_fDecayTime, fMaxHFRatio, in_uSampleRate, in_uDelayLineLength[ i ] );
		}
		return AK_Success;
	}

	/** 2012 0x98e7d0. */
	void FDN4::Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			FDNDelayLine[ i ].Term( in_pAllocator );
		}
	}

	/** 2012 0x98e800. */
	void FDN4::Reset()
	{
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			FDNDelayLine[ i ].Reset();
		}
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			delayLowPassFilter[ i ].fFFbk1 = 0.f;
		}
	}

	/** The read half every ProcessBufferAccum inlines: each line's oldest sample through its damping one-pole. */
	void FDN4::ReadLines( AkReal32 out_fLines[ 4 ] )
	{
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			const AkReal32 fRead = FDNDelayLine[ i ].pfDelay[ FDNDelayLine[ i ].uCurOffset ];
			out_fLines[ i ] = delayLowPassFilter[ i ].fB0 * fRead - delayLowPassFilter[ i ].fA1 * delayLowPassFilter[ i ].fFFbk1;
			delayLowPassFilter[ i ].fFFbk1 = out_fLines[ i ];
		}
	}

	/** The write half: Householder feedback (-1/2 of the sum added to each line), rotated by one line, plus the input. */
	void FDN4::WriteLines( const AkReal32 in_fLines[ 4 ], AkReal32 in_fIn )
	{
		const AkReal32 fMix = ( ( ( in_fLines[ 3 ] + in_fLines[ 2 ] ) + in_fLines[ 1 ] ) + in_fLines[ 0 ] ) * -0.5f;
		const AkReal32 aFeedback[ 4 ] = { in_fLines[ 1 ] + fMix, in_fLines[ 2 ] + fMix, in_fLines[ 3 ] + fMix, in_fLines[ 0 ] + fMix };
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			DelayLine & rLine = FDNDelayLine[ i ];
			rLine.pfDelay[ rLine.uCurOffset ] = aFeedback[ i ] + in_fIn;
			if( ++rLine.uCurOffset == rLine.uDelayLineLength )
			{
				rLine.uCurOffset = 0;
			}
		}
	}

	/** 2012 0x98e840: one output (+ - + -). */
	void FDN4::ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer, AkUInt32 in_uNumFrames )
	{
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			AkReal32 d[ 4 ];
			ReadLines( d );
			io_pfOutBuffer[ i ] = ( ( ( d[ 0 ] - d[ 1 ] ) + d[ 2 ] ) - d[ 3 ] ) + io_pfOutBuffer[ i ];
			WriteLines( d, in_pfInBuffer[ i ] );
		}
	}

	/** 2012 0x98ea20: two decorrelated outputs (+ - + -, + + - -). */
	void FDN4::ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer1, AkReal32 * io_pfOutBuffer2, AkUInt32 in_uNumFrames )
	{
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			AkReal32 d[ 4 ];
			ReadLines( d );
			io_pfOutBuffer1[ i ] = ( ( ( d[ 0 ] - d[ 1 ] ) + d[ 2 ] ) - d[ 3 ] ) + io_pfOutBuffer1[ i ];
			io_pfOutBuffer2[ i ] = ( ( ( d[ 1 ] + d[ 0 ] ) - d[ 2 ] ) - d[ 3 ] ) + io_pfOutBuffer2[ i ];
			WriteLines( d, in_pfInBuffer[ i ] );
		}
	}

	/** 2012 0x98ec40: three outputs (+ - + -, + + - -, + - - +). */
	void FDN4::ProcessBufferAccum( AkReal32 * in_pfInBuffer, AkReal32 * io_pfOutBuffer1, AkReal32 * io_pfOutBuffer2, AkReal32 * io_pfOutBuffer3, AkUInt32 in_uNumFrames )
	{
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			AkReal32 d[ 4 ];
			ReadLines( d );
			io_pfOutBuffer1[ i ] = ( ( ( d[ 0 ] - d[ 1 ] ) + d[ 2 ] ) - d[ 3 ] ) + io_pfOutBuffer1[ i ];
			io_pfOutBuffer3[ i ] = ( ( ( d[ 0 ] - d[ 1 ] ) - d[ 2 ] ) + io_pfOutBuffer3[ i ] ) + d[ 3 ];
			io_pfOutBuffer2[ i ] = ( ( ( d[ 1 ] + d[ 0 ] ) - d[ 2 ] ) - d[ 3 ] ) + io_pfOutBuffer2[ i ];
			WriteLines( d, in_pfInBuffer[ i ] );
		}
	}
}
