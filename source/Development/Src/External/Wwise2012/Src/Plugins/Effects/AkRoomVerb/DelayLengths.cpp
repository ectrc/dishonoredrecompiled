/*=============================================================================
	DelayLengths.cpp - see DelayLengths.h. Agent IH4 fork F1 (wave 26).
=============================================================================*/
#include "DelayLengths.h"
#include <math.h>
#include <stdlib.h>

namespace DelayLengths
{
	/** The SDK's random generator: x = 196314165 x + 907633515 (mod 2^32), a draw is x / 2^32. */
	static inline double AkNextRandom( AkUInt32 & io_uSeed )
	{
		io_uSeed = 196314165u * io_uSeed + 907633515u;
		return (double)io_uSeed * 2.3283064e-10;
	}

	/** 2012 0x9886b0. */
	static int __cdecl AkDelayQSortCompareFloat( const void * a, const void * b )
	{
		const AkReal32 fDiff = *(const AkReal32 *)a - *(const AkReal32 *)b;
		if( fDiff < 0.f )
		{
			return -1;
		}
		return fDiff > 0.f;
	}

	/** 2012 0x9886e0: lines spread evenly between the room-shape fraction of the density delay and its mirror, each
	    jittered by up to the random percentage, sorted. */
	void ComputeFDNDelayTimes( const AlgorithmTunings & in_Tunings, AkReal32 in_fDensity, AkReal32 in_fRoomShape, AkUInt32 in_uNumDelayLines, AkReal32 * out_pfFDNDelayTimes )
	{
		const AkReal32 fDensityDelay = ( in_Tunings.fDensityDelayMax - in_Tunings.fDensityDelayMin ) * in_fDensity * 0.0099999998f + in_Tunings.fDensityDelayMin;
		const AkReal32 fMinTime = ( ( in_Tunings.fRoomShapeMax - in_Tunings.fRoomShapeMin ) * in_fRoomShape * 0.0099999998f + in_Tunings.fRoomShapeMin ) * fDensityDelay;
		const AkReal32 fMaxTime = ( fDensityDelay - fMinTime ) + fDensityDelay;
		const double dRange = (AkReal32)( fMaxTime - fMinTime );
		AkUInt32 uSeed = 7032007;
		for( AkUInt32 i = 0; i < in_uNumDelayLines; ++i )
		{
			const double dTime = (double)i * dRange / (double)( in_uNumDelayLines - 1 ) + fMinTime;
			out_pfFDNDelayTimes[ i ] = (AkReal32)( dTime + AkNextRandom( uSeed ) * ( in_Tunings.fDensityDelayRdmPerc * 0.0099999998f ) * dTime );
		}
		qsort( out_pfFDNDelayTimes, in_uNumDelayLines, sizeof( AkReal32 ), AkDelayQSortCompareFloat );
	}

	/** 2012 0x988980: the reverb unit input delay divided among the units, each jittered. */
	void ComputeFDNInputDelayTimes( const AlgorithmTunings & in_Tunings, AkUInt32 in_uNumDelayLines, AkReal32 * out_pfFDNInputDelayTimes )
	{
		AkUInt32 uSeed = 29101977;
		const double dTime = in_Tunings.fReverbUnitInputDelay * ( 1.0 / (double)in_uNumDelayLines );
		for( AkUInt32 i = 0; i < in_uNumDelayLines; ++i )
		{
			out_pfFDNInputDelayTimes[ i ] = (AkReal32)( dTime + AkNextRandom( uSeed ) * ( in_Tunings.fReverbUnitInputDelayRmdPerc * 0.0099999998f ) * dTime );
		}
	}

	/** 2012 0x988b20: the longest allpass first, each next one scaled by the percentage, each jittered, sorted. */
	void ComputeDiffusionFiltersDelayTimes( const AlgorithmTunings & in_Tunings, AkUInt32 in_uNumReverbUnits, AkReal32 * out_pfAPFDelayTimes )
	{
		AkUInt32 uSeed = 3072007;
		AkReal32 fLength = in_Tunings.fDiffusionDelayMax;
		for( AkUInt32 i = 0; i < in_uNumReverbUnits; ++i )
		{
			const AkReal32 fThisLength = fLength;
			out_pfAPFDelayTimes[ i ] = (AkReal32)( fThisLength + AkNextRandom( uSeed ) * ( in_Tunings.fDiffusionDelayRdmPerc * 0.0099999998f ) * fThisLength );
			fLength = fLength * in_Tunings.fDiffusionDelayScalePerc * 0.0099999998f;
		}
		qsort( out_pfAPFDelayTimes, in_uNumReverbUnits, sizeof( AkReal32 ), AkDelayQSortCompareFloat );
	}

	/** 2012 0x988d00: the next odd number with no odd divisor below sqrt(n)+1 (the bound is computed once), different
	    from the previous line's length. */
	void MakePrimeNumber( AkUInt32 & io_uIn, AkUInt32 in_uPrevVal )
	{
		if( !( io_uIn & 1 ) )
		{
			++io_uIn;
		}
		const AkUInt32 uBound = (AkUInt32)(AkInt32)sqrt( (double)io_uIn ) + 1;
		for( ;; )
		{
			bool bComposite = false;
			for( AkUInt32 uDiv = 3; uDiv < uBound; uDiv += 2 )
			{
				if( !( io_uIn % uDiv ) )
				{
					bComposite = true;
					break;
				}
			}
			if( !bComposite && io_uIn != in_uPrevVal )
			{
				return;
			}
			io_uIn += 2;
		}
	}

	/** 2012 0x988d70. */
	void ComputePrimeDelayLengths( AkUInt32 in_uSampleRate, AkUInt32 * out_puDelayLengths, const AkReal32 * in_pfDelayTimes, AkUInt32 in_uNumDelays )
	{
		const AkReal32 fSampleRate = (AkReal32)in_uSampleRate;
		for( AkUInt32 i = 0; i < in_uNumDelays; ++i )
		{
			out_puDelayLengths[ i ] = (AkUInt32)(AkInt64)( in_pfDelayTimes[ i ] * 0.001 * fSampleRate );
			MakePrimeNumber( out_puDelayLengths[ i ], i ? out_puDelayLengths[ i - 1 ] : 0 );
		}
	}
}
