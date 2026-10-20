/*=============================================================================
	MultiChannelBiquadFilter.cpp - AK::DSP::MultiChannelBiquadFilter<6>, Wwise 2012.1 (plugins/common/multichannelbiquadfilter.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes; retail instantiates only <6>
	(the pitch shifter and the harmonizer), so the template is explicitly instantiated for 6 here. The frequency is
	clamped to 0.45 x the sample rate; the shelves use a slope of 1 (retail's sqrt((A + 1/A) * 0 + 2) term kept).
	Retail evaluates pow/sin/cos/sqrt/tan in double and the rest in float; the same split is kept.
=============================================================================*/
#include "MultiChannelBiquadFilter.h"
#include <math.h>

namespace AK
{
namespace DSP
{

/** 2012 0x9904e0 (no 2013 twin: inlined). */
template < AkUInt32 MAXCHANNELS >
MultiChannelBiquadFilter<MAXCHANNELS>::Memories::Memories()
	: fFFwd1( 0.f ), fFFwd2( 0.f ), fFFbk1( 0.f ), fFFbk2( 0.f )
{
}

/** 2012 0x991960, 2013 0x98d260. */
template < AkUInt32 MAXCHANNELS >
void MultiChannelBiquadFilter<MAXCHANNELS>::ComputeCoefs( FilterType in_eFilterType, AkUInt32 in_uSampleRate, AkReal32 in_fFreq, AkReal32 in_fGain, AkReal32 in_fQ )
{
	const double dSampleRate = (double)in_uSampleRate;
	const AkReal32 fSampleRate = (AkReal32)dSampleRate;
	if( in_fFreq >= fSampleRate * 0.45f )
	{
		in_fFreq = fSampleRate * 0.45f;
	}
	AkReal32 fB0, fB1, fB2, fA0, fA1, fA2;
	switch( in_eFilterType )
	{
	case FilterType_LowShelf:
		{
			const double dLinAmp = pow( 10.0, in_fGain * 0.025f );
			const AkReal32 fA = (AkReal32)dLinAmp;
			const AkReal32 fAMinus1 = fA - 1.f;
			const AkReal32 fOmega = in_fFreq * 6.2831855f / fSampleRate;
			const AkReal32 fAPlus1 = fA + 1.f;
			const AkReal32 fCosOmega = (AkReal32)cos( fOmega );
			const AkReal32 fAMinus1Cos = fAMinus1 * fCosOmega;
			const AkReal32 fTmp = fAPlus1 - fAMinus1Cos;
			fA1 = ( ( fAPlus1 * fCosOmega ) + fAMinus1 ) * -2.f;
			fB1 = ( ( fAMinus1 - ( fAPlus1 * fCosOmega ) ) * fA ) * 2.f;
			const AkReal32 fTwoSqrtATimesAlpha = (AkReal32)( sqrt( ( 1.0 / dLinAmp + dLinAmp ) * 0.0 + 2.0 ) * sin( fOmega ) * 0.5 * sqrt( dLinAmp ) * 2.0 );
			fB0 = ( fTmp + fTwoSqrtATimesAlpha ) * fA;
			fB2 = ( fTmp - fTwoSqrtATimesAlpha ) * fA;
			fA0 = ( fAMinus1Cos + fTwoSqrtATimesAlpha ) + fAPlus1;
			fA2 = ( fAMinus1Cos + fAPlus1 ) - fTwoSqrtATimesAlpha;
		}
		break;
	case FilterType_Peaking:
		{
			const AkReal32 fOmega = (AkReal32)( in_fFreq * 6.2831855f / dSampleRate );
			const AkReal32 fCosOmega = (AkReal32)cos( fOmega );
			const AkReal32 fLinAmp = (AkReal32)pow( 10.0, in_fGain * 0.025f );
			fB1 = fCosOmega * -2.f;
			const AkReal32 fAlpha = (AkReal32)( sin( fOmega ) / ( in_fQ + in_fQ ) );
			fB2 = 1.f - ( fAlpha * fLinAmp );
			fB0 = ( fAlpha * fLinAmp ) + 1.f;
			fA0 = ( fAlpha / fLinAmp ) + 1.f;
			fA1 = fCosOmega * -2.f;
			fA2 = 1.f - ( fAlpha / fLinAmp );
		}
		break;
	case FilterType_HighShelf:
		{
			const double dLinAmp = pow( 10.0, in_fGain * 0.025f );
			const AkReal32 fA = (AkReal32)dLinAmp;
			const AkReal32 fOmega = in_fFreq * 6.2831855f / fSampleRate;
			const AkReal32 fCosOmega = (AkReal32)cos( fOmega );
			const AkReal32 fAMinus1Cos = ( fA - 1.f ) * fCosOmega;
			const AkReal32 fAPlus1Cos = ( fA + 1.f ) * fCosOmega;
			fA1 = ( ( fA - 1.f ) - fAPlus1Cos ) * 2.f;
			fB1 = ( ( fAPlus1Cos + ( fA - 1.f ) ) * fA ) * -2.f;
			const AkReal32 fTwoSqrtATimesAlpha = (AkReal32)( sqrt( ( 1.0 / dLinAmp + dLinAmp ) * 0.0 + 2.0 ) * sin( fOmega ) * 0.5 * sqrt( dLinAmp ) * 2.0 );
			fB0 = ( ( fAMinus1Cos + fTwoSqrtATimesAlpha ) + ( fA + 1.f ) ) * fA;
			const AkReal32 fSum = fAMinus1Cos + ( fA + 1.f );
			const AkReal32 fDiff = ( fA + 1.f ) - fAMinus1Cos;
			fB2 = ( fSum - fTwoSqrtATimesAlpha ) * fA;
			fA0 = fDiff + fTwoSqrtATimesAlpha;
			fA2 = fDiff - fTwoSqrtATimesAlpha;
		}
		break;
	case FilterType_HighPass:
		{
			const AkReal32 fC = (AkReal32)tan( in_fFreq * 3.1415927f / dSampleRate );
			const AkReal32 fCSquared = fC * fC;
			const AkReal32 fCSquaredPlus1 = fCSquared + 1.f;
			const AkReal32 fAlpha = fC * 1.4142135f;
			fB0 = 1.f / ( fCSquaredPlus1 + fAlpha );
			fB1 = fB0 * -2.f;
			fA1 = -( ( fCSquared - 1.f ) * ( fB0 * -2.f ) );
			fA2 = ( fCSquaredPlus1 - fAlpha ) * fB0;
			fA0 = 1.f;
			fB2 = fB0;
		}
		break;
	case FilterType_BandPass:
		{
			fB1 = 0.f;
			const AkReal32 fOmega = (AkReal32)( in_fFreq * 6.2831855f / dSampleRate );
			const AkReal32 fAlpha = (AkReal32)( sin( fOmega ) / ( in_fQ + in_fQ ) );
			fB0 = fAlpha;
			fB2 = -fAlpha;
			fA0 = fAlpha + 1.f;
			fA1 = (AkReal32)( cos( fOmega ) * -2.0 );
			fA2 = 1.f - fAlpha;
		}
		break;
	case FilterType_Notch:
		{
			fB0 = 1.f;
			fB2 = 1.f;
			const AkReal32 fOmega = (AkReal32)( in_fFreq * 6.2831855f / dSampleRate );
			const AkReal32 fAlpha = (AkReal32)( sin( fOmega ) / ( in_fQ + in_fQ ) );
			fB1 = (AkReal32)( cos( fOmega ) * -2.0 );
			fA0 = fAlpha + 1.f;
			fA1 = fB1;
			fA2 = 1.f - fAlpha;
		}
		break;
	default:
		{
			const AkReal32 fC = (AkReal32)( 1.0 / tan( in_fFreq * 3.1415927f / dSampleRate ) );
			const AkReal32 fCSquared = fC * fC;
			const AkReal32 fCSquaredPlus1 = fCSquared + 1.f;
			const AkReal32 fAlpha = fC * 1.4142135f;
			fB0 = 1.f / ( fCSquaredPlus1 + fAlpha );
			fB1 = fB0 * 2.f;
			fA1 = ( 1.f - fCSquared ) * ( fB0 * 2.f );
			fA2 = ( fCSquaredPlus1 - fAlpha ) * fB0;
			fA0 = 1.f;
			fB2 = fB0;
		}
		break;
	}
	const AkReal32 fNorm = 1.f / fA0;
	m_Coefficients.fB1 = fNorm * fB1;
	m_Coefficients.fB0 = fNorm * fB0;
	m_Coefficients.fB2 = fNorm * fB2;
	m_Coefficients.fA1 = -( fNorm * fA1 );
	m_Coefficients.fA2 = -( fNorm * fA2 );
}

/** 2012 0x991e40, 2013 0x98d740. */
template < AkUInt32 MAXCHANNELS >
void MultiChannelBiquadFilter<MAXCHANNELS>::Reset()
{
	for( AkUInt32 i = 0; i < MAXCHANNELS; ++i )
	{
		m_Memories[ i ].fFFbk1 = 0.f;
		m_Memories[ i ].fFFbk2 = 0.f;
		m_Memories[ i ].fFFwd1 = 0.f;
		m_Memories[ i ].fFFwd2 = 0.f;
	}
}

/** 2012 0x991ec0, 2013 0x98d7c0: y = ((((b0 x + b2 x2) + b1 x1) + a2 y2) + a1 y1), retail's summation order. */
template < AkUInt32 MAXCHANNELS >
void MultiChannelBiquadFilter<MAXCHANNELS>::ProcessChannel( AkReal32 * io_pfBuffer, AkUInt32 in_uNumframes, AkUInt32 in_uChannelIndex )
{
	Memories & rMems = m_Memories[ in_uChannelIndex ];
	AkReal32 fFFwd1 = rMems.fFFwd1;
	AkReal32 fFFwd2 = rMems.fFFwd2;
	AkReal32 fFFbk1 = rMems.fFFbk1;
	AkReal32 fFFbk2 = rMems.fFFbk2;
	const AkReal32 fB0 = m_Coefficients.fB0;
	const AkReal32 fB1 = m_Coefficients.fB1;
	const AkReal32 fB2 = m_Coefficients.fB2;
	const AkReal32 fA1 = m_Coefficients.fA1;
	const AkReal32 fA2 = m_Coefficients.fA2;
	AkReal32 * pfEnd = io_pfBuffer + in_uNumframes;
	while( io_pfBuffer < pfEnd )
	{
		const AkReal32 fIn = *io_pfBuffer;
		const AkReal32 fOut = ( fA1 * fFFbk1 ) + ( ( fA2 * fFFbk2 ) + ( ( fB1 * fFFwd1 ) + ( ( fB0 * fIn ) + ( fB2 * fFFwd2 ) ) ) );
		*io_pfBuffer++ = fOut;
		fFFwd2 = fFFwd1;
		fFFbk2 = fFFbk1;
		fFFwd1 = fIn;
		fFFbk1 = fOut;
	}
	rMems.fFFwd1 = fFFwd1;
	rMems.fFFwd2 = fFFwd2;
	rMems.fFFbk1 = fFFbk1;
	rMems.fFFbk2 = fFFbk2;
}

template class MultiChannelBiquadFilter<6>;

}
}
