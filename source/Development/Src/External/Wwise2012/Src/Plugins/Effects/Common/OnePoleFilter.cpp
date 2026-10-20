/*=============================================================================
	OnePoleFilter.cpp - DSP::OnePoleFilter (2012 0x99b590..0x99b740, 2013 0x9969c0..) and DSP::OnePoleZeroHPFilter
	(0x98f470). Agent IH4 fork F1 (wave 26); deviation named in OnePoleFilter.h (no 4x unroll).
=============================================================================*/
#include "OnePoleFilter.h"
#include <math.h>

namespace DSP
{
	/** 2012 0x99b590. */
	void OnePoleFilter::ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames )
	{
		AkReal32 fFbk1 = fFFbk1;
		const AkReal32 fLocalB0 = fB0;
		const AkReal32 fLocalA1 = fA1;
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			fFbk1 = io_pfBuffer[ i ] * fLocalB0 - fLocalA1 * fFbk1;
			io_pfBuffer[ i ] = fFbk1;
		}
		if( in_uNumFrames )
		{
			fFFbk1 = fFbk1;
		}
	}

	/** 2012 0x99b680: the SDK's one-pole design from the cutoff (none = pass-through). */
	void OnePoleFilter::ComputeCoefs( FilterType eType, AkReal32 in_fFc, AkUInt32 in_uSr, AkReal32 & out_fB0, AkReal32 & out_fA1 )
	{
		if( eType == FILTERCURVETYPE_NONE )
		{
			out_fA1 = 0.f;
			out_fB0 = 1.f;
		}
		else if( eType == FILTERCURVETYPE_LOWPASS )
		{
			const double fC = 2.0 - cos( in_fFc / (double)in_uSr * 6.2831855f );
			const double fA = sqrt( fC * fC - 1.0 ) - fC;
			out_fA1 = (AkReal32)fA;
			out_fB0 = (AkReal32)( fA + 1.0 );
		}
		else if( eType == FILTERCURVETYPE_HIGHPASS )
		{
			const double fC = cos( in_fFc / (double)in_uSr * 6.2831855f ) + 2.0;
			const double fA = fC - sqrt( fC * fC - 1.0 );
			out_fA1 = (AkReal32)fA;
			out_fB0 = (AkReal32)( 1.0 - fA );
		}
	}

	/** 2012 0x99b740. */
	void OnePoleFilter::SetCoefs( FilterType eType, AkReal32 in_fFc, AkUInt32 in_uSr )
	{
		ComputeCoefs( eType, in_fFc, in_uSr, fB0, fA1 );
	}

	/** 2012 0x98f470. */
	void OnePoleZeroHPFilter::ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames )
	{
		AkReal32 fFFwd1 = m_fFFwd1;
		AkReal32 fFbk1 = m_fFbk1;
		const AkReal32 fA1 = m_fA1;
		for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
		{
			const AkReal32 fIn = io_pfBuffer[ i ];
			fFbk1 = fFbk1 * fA1 + ( fIn - fFFwd1 );
			io_pfBuffer[ i ] = fFbk1;
			fFFwd1 = fIn;
		}
		m_fFFwd1 = fFFwd1;
		m_fFbk1 = fFbk1;
	}
}
