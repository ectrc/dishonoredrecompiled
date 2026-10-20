/*=============================================================================
	BiquadFilter.cpp - DSP::BiquadFilter (2012 0x98d370..0x98d870, ctor 0x98bd40), DSP::Mix2Interp (0x994a70,
	0x994c60, 0x994e40), DSP::Mix3Interp (0x98f550, 0x98f8d0); 2013 matched by bytes. Agent IH4 fork F1 (wave 26).
	Deviations named in BiquadFilter.h.
=============================================================================*/
#include "BiquadFilter.h"
#include <math.h>

namespace DSP
{
	/** 2012 0x98d370: the RBJ cookbook designs (shelf slope 1), Butterworth low/high pass from tan; the frequency is
	    clamped to 0.45 fs; a1 a2 are stored negated so ProcessBuffer only adds. Gain in dB (A = 10^(dB/40)). */
	void BiquadFilter::ComputeCoefs( FilterType in_eFilterType, AkUInt32 in_uSampleRate, AkReal32 in_fFreq, AkReal32 in_fGain, AkReal32 in_fQ )
	{
		const AkReal32 fSampleRate = (AkReal32)in_uSampleRate;
		if( in_fFreq >= fSampleRate * 0.45f )
		{
			in_fFreq = fSampleRate * 0.45f;
		}
		AkReal32 fB0, fB1, fB2, fA0, fA1, fA2;
		switch( in_eFilterType )
		{
		case FilterType_LowShelf:
		{
			const double dA = pow( 10.0, in_fGain * 0.025 );
			const AkReal32 fA = (AkReal32)dA;
			const AkReal32 fAm1 = fA - 1.f;
			const AkReal32 fAp1 = fA + 1.f;
			const double dOmega = in_fFreq * 6.2831855 / fSampleRate;
			const AkReal32 fCos = (AkReal32)cos( dOmega );
			const AkReal32 fTwoSqrtAAlpha = (AkReal32)( sqrt( ( 1.0 / dA + dA ) * 0.0 + 2.0 ) * sin( dOmega ) * 0.5 * sqrt( dA ) * 2.0 );
			const AkReal32 fAm1Cos = fAm1 * fCos;
			const AkReal32 fT = fAp1 - fAm1Cos;
			fB0 = ( fT + fTwoSqrtAAlpha ) * fA;
			fB1 = ( ( fAm1 - fAp1 * fCos ) * fA ) * 2.f;
			fB2 = ( fT - fTwoSqrtAAlpha ) * fA;
			fA0 = ( fAm1Cos + fTwoSqrtAAlpha ) + fAp1;
			fA1 = ( fAp1 * fCos + fAm1 ) * -2.f;
			fA2 = ( fAm1Cos + fAp1 ) - fTwoSqrtAAlpha;
			break;
		}
		case FilterType_Peaking:
		{
			const double dOmega = in_fFreq * 6.2831855 / fSampleRate;
			const AkReal32 fCos = (AkReal32)cos( dOmega );
			const AkReal32 fA = (AkReal32)pow( 10.0, in_fGain * 0.025 );
			const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( in_fQ + in_fQ ) );
			fB0 = fAlpha * fA + 1.f;
			fB1 = fCos * -2.f;
			fB2 = 1.f - fAlpha * fA;
			fA0 = fAlpha / fA + 1.f;
			fA1 = fCos * -2.f;
			fA2 = 1.f - fAlpha / fA;
			break;
		}
		case FilterType_HighShelf:
		{
			const double dA = pow( 10.0, in_fGain * 0.025 );
			const AkReal32 fA = (AkReal32)dA;
			const double dOmega = in_fFreq * 6.2831855 / fSampleRate;
			const AkReal32 fCos = (AkReal32)cos( dOmega );
			const AkReal32 fAm1Cos = ( fA - 1.f ) * fCos;
			const AkReal32 fAp1Cos = ( fA + 1.f ) * fCos;
			const AkReal32 fTwoSqrtAAlpha = (AkReal32)( sqrt( ( 1.0 / dA + dA ) * 0.0 + 2.0 ) * sin( dOmega ) * 0.5 * sqrt( dA ) * 2.0 );
			fB0 = ( ( fAm1Cos + fTwoSqrtAAlpha ) + ( fA + 1.f ) ) * fA;
			fB1 = ( ( fAp1Cos + ( fA - 1.f ) ) * fA ) * -2.f;
			fB2 = ( ( fAm1Cos + ( fA + 1.f ) ) - fTwoSqrtAAlpha ) * fA;
			fA0 = ( ( fA + 1.f ) - fAm1Cos ) + fTwoSqrtAAlpha;
			fA1 = ( ( fA - 1.f ) - fAp1Cos ) * 2.f;
			fA2 = ( ( fA + 1.f ) - fAm1Cos ) - fTwoSqrtAAlpha;
			break;
		}
		case FilterType_HighPass:
		{
			const AkReal32 fK = (AkReal32)tan( in_fFreq * 3.1415927 / fSampleRate );
			const AkReal32 fK2 = fK * fK;
			const AkReal32 fKSqrt2 = fK * 1.4142135f;
			fB0 = 1.f / ( ( fK2 + 1.f ) + fKSqrt2 );
			fB1 = fB0 * -2.f;
			fB2 = fB0;
			fA0 = 1.f;
			fA1 = -( ( fK2 - 1.f ) * fB1 );
			fA2 = ( ( fK2 + 1.f ) - fKSqrt2 ) * fB0;
			break;
		}
		case FilterType_BandPass:
		{
			const double dOmega = in_fFreq * 6.2831855 / fSampleRate;
			const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( in_fQ + in_fQ ) );
			fB0 = fAlpha;
			fB1 = 0.f;
			fB2 = -fAlpha;
			fA0 = fAlpha + 1.f;
			fA1 = (AkReal32)( cos( dOmega ) * -2.0 );
			fA2 = 1.f - fAlpha;
			break;
		}
		case FilterType_Notch:
		{
			const double dOmega = in_fFreq * 6.2831855 / fSampleRate;
			const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( in_fQ + in_fQ ) );
			fB0 = 1.f;
			fB1 = (AkReal32)( cos( dOmega ) * -2.0 );
			fB2 = 1.f;
			fA0 = fAlpha + 1.f;
			fA1 = fB1;
			fA2 = 1.f - fAlpha;
			break;
		}
		default:
		{
			const AkReal32 fK = (AkReal32)( 1.0 / tan( in_fFreq * 3.1415927 / fSampleRate ) );
			const AkReal32 fK2 = fK * fK;
			const AkReal32 fKSqrt2 = fK * 1.4142135f;
			fB0 = 1.f / ( ( fK2 + 1.f ) + fKSqrt2 );
			fB1 = fB0 * 2.f;
			fB2 = fB0;
			fA0 = 1.f;
			fA1 = ( 1.f - fK2 ) * fB1;
			fA2 = ( ( fK2 + 1.f ) - fKSqrt2 ) * fB0;
			break;
		}
		}
		const AkReal32 fNorm = 1.f / fA0;
		m_Coefficients.fB1 = fNorm * fB1;
		m_Coefficients.fB0 = fNorm * fB0;
		m_Coefficients.fB2 = fNorm * fB2;
		m_Coefficients.fA1 = -( fNorm * fA1 );
		m_Coefficients.fA2 = -( fNorm * fA2 );
	}

	/** 2012 0x98d850. */
	void BiquadFilter::Reset()
	{
		m_Memories.fFFwd1 = 0.f;
		m_Memories.fFFwd2 = 0.f;
		m_Memories.fFFbk1 = 0.f;
		m_Memories.fFFbk2 = 0.f;
	}

	/** 2012 0x98d870: direct form I, y = ((b0 x + b2 x2) + b1 x1) + a2 y2 + a1 y1 (a negated). */
	void BiquadFilter::ProcessBuffer( AkReal32 * io_pBuffer, AkUInt32 in_uNumframes )
	{
		Memories mems = m_Memories;
		const Coefficients & c = m_Coefficients;
		for( AkUInt32 i = 0; i < in_uNumframes; ++i )
		{
			const AkReal32 fIn = io_pBuffer[ i ];
			AkReal32 fOut = c.fB0 * fIn + c.fB2 * mems.fFFwd2;
			mems.fFFwd2 = mems.fFFwd1;
			fOut = c.fB1 * mems.fFFwd1 + fOut;
			mems.fFFwd1 = fIn;
			fOut = fOut + c.fA2 * mems.fFFbk2;
			mems.fFFbk2 = mems.fFFbk1;
			mems.fFFbk1 = c.fA1 * mems.fFFbk1 + fOut;
			io_pBuffer[ i ] = mems.fFFbk1;
		}
		m_Memories = mems;
	}

	/** 2012 0x994a70: gains ramp linearly over the buffer (constant when both are at target). */
	void Mix2Interp( AkReal32 * AK_RESTRICT in_pfInput1, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT out_pfOutput,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkUInt32 in_uNumFrames )
	{
		if( in_fTargetGain1 == in_fCurrentGain1 && in_fTargetGain2 == in_fCurrentGain2 )
		{
			for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
			{
				out_pfOutput[ i ] = in_pfInput1[ i ] * in_fTargetGain1 + in_pfInput2[ i ] * in_fTargetGain2;
			}
			return;
		}
		const AkReal32 fInc1 = ( in_fTargetGain1 - in_fCurrentGain1 ) / (AkReal32)in_uNumFrames;
		const AkReal32 fInc2 = ( in_fTargetGain2 - in_fCurrentGain2 ) / (AkReal32)in_uNumFrames;
		AkReal32 fGain1 = in_fCurrentGain1;
		AkReal32 fGain2 = in_fCurrentGain2;
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			out_pfOutput[ i ] = in_pfInput2[ i ] * fGain2 + fGain1 * in_pfInput1[ i ];
			fGain1 = fInc1 + fGain1;
			fGain2 = fInc2 + fGain2;
		}
	}

	/** 2012 0x994c60: in place into the first input. */
	void Mix2Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkUInt32 in_uNumFrames )
	{
		Mix2Interp( io_pfIn1Out, in_pfInput2, io_pfIn1Out, in_fCurrentGain1, in_fTargetGain1, in_fCurrentGain2, in_fTargetGain2, in_uNumFrames );
	}

	/** 2012 0x994e40: constant gains, in place. */
	void Mix2Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 in_fGain1, AkReal32 in_fGain2, AkUInt32 in_uNumFrames )
	{
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			io_pfIn1Out[ i ] = io_pfIn1Out[ i ] * in_fGain1 + in_pfInput2[ i ] * in_fGain2;
		}
	}

	/** 2012 0x98f550. */
	void Mix3Interp( AkReal32 * AK_RESTRICT in_pfInput1, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT in_pfInput3, AkReal32 * AK_RESTRICT out_pfOutput,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkReal32 in_fCurrentGain3, AkReal32 in_fTargetGain3, AkUInt32 in_uNumFrames )
	{
		if( in_fTargetGain1 == in_fCurrentGain1 && in_fTargetGain2 == in_fCurrentGain2 && in_fTargetGain3 == in_fCurrentGain3 )
		{
			for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
			{
				out_pfOutput[ i ] = ( in_pfInput1[ i ] * in_fTargetGain1 + in_pfInput2[ i ] * in_fTargetGain2 ) + in_pfInput3[ i ] * in_fTargetGain3;
			}
			return;
		}
		const AkReal32 fInc1 = ( in_fTargetGain1 - in_fCurrentGain1 ) / (AkReal32)in_uNumFrames;
		const AkReal32 fInc2 = ( in_fTargetGain2 - in_fCurrentGain2 ) / (AkReal32)in_uNumFrames;
		const AkReal32 fInc3 = ( in_fTargetGain3 - in_fCurrentGain3 ) / (AkReal32)in_uNumFrames;
		AkReal32 fGain1 = in_fCurrentGain1;
		AkReal32 fGain2 = in_fCurrentGain2;
		AkReal32 fGain3 = in_fCurrentGain3;
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			out_pfOutput[ i ] = ( in_pfInput1[ i ] * fGain1 + in_pfInput2[ i ] * fGain2 ) + in_pfInput3[ i ] * fGain3;
			fGain1 = fInc1 + fGain1;
			fGain2 = fInc2 + fGain2;
			fGain3 = fInc3 + fGain3;
		}
	}

	/** 2012 0x98f8d0: in place into the first input. */
	void Mix3Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT in_pfInput3,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkReal32 in_fCurrentGain3, AkReal32 in_fTargetGain3, AkUInt32 in_uNumFrames )
	{
		Mix3Interp( io_pfIn1Out, in_pfInput2, in_pfInput3, io_pfIn1Out, in_fCurrentGain1, in_fTargetGain1, in_fCurrentGain2, in_fTargetGain2, in_fCurrentGain3, in_fTargetGain3, in_uNumFrames );
	}
}
