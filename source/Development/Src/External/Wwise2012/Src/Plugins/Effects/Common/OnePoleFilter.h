/*=============================================================================
	OnePoleFilter.h - DSP::OnePoleFilter and DSP::OnePoleZeroHPFilter (plugins\effects\common\onepolefilter.cpp,
	onepolezerohpfilter.cpp), Wwise 2012.1.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (12 bytes each). Deviation (named, all
	helpers of this folder): retail's ProcessBuffer loops are unrolled by four; the port runs the same per-sample
	arithmetic in one loop (bit-identical output).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

namespace DSP
{
	/** y[n] = b0 x[n] - a1 y[n-1]. */
	class OnePoleFilter
	{
	public:
		enum FilterType
		{
			FILTERCURVETYPE_NONE		= 0,
			FILTERCURVETYPE_LOWPASS		= 1,
			FILTERCURVETYPE_HIGHPASS	= 2
		};

		OnePoleFilter() : fFFbk1( 0.f ), fB0( 1.f ), fA1( 0.f ) {}

		void Reset() { fFFbk1 = 0.f; }
		void ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames );
		void SetCoefs( FilterType eType, AkReal32 in_fFc, AkUInt32 in_uSr );
		void SetCoefs( AkReal32 in_fB0, AkReal32 in_fA1 ) { fB0 = in_fB0; fA1 = in_fA1; }
		static void ComputeCoefs( FilterType eType, AkReal32 in_fFc, AkUInt32 in_uSr, AkReal32 & out_fB0, AkReal32 & out_fA1 );

		AkReal32 fFFbk1;
		AkReal32 fB0;
		AkReal32 fA1;
	};

	/** DC blocker: y[n] = x[n] - x[n-1] + a1 y[n-1]. */
	class OnePoleZeroHPFilter
	{
	public:
		OnePoleZeroHPFilter() : m_fFFwd1( 0.f ), m_fFbk1( 0.f ), m_fA1( 0.f ) {}

		void Reset() { m_fFFwd1 = 0.f; m_fFbk1 = 0.f; }
		void SetCoefs( AkReal32 in_fA1 ) { m_fA1 = in_fA1; }
		void ProcessBuffer( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames );

		AkReal32 m_fFFwd1;
		AkReal32 m_fFbk1;
		AkReal32 m_fA1;
	};
}

static_assert( sizeof( DSP::OnePoleFilter ) == 12, "DSP::OnePoleFilter: 2012 PDB sizeof 12" );
static_assert( sizeof( DSP::OnePoleZeroHPFilter ) == 12, "DSP::OnePoleZeroHPFilter: 2012 PDB sizeof 12" );
