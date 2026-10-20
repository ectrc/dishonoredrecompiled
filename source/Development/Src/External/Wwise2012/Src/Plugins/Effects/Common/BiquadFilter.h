/*=============================================================================
	BiquadFilter.h - DSP::BiquadFilter (plugins\effects\common\biquadfilter.cpp/.h) and the gain-interpolated mixers
	DSP::Mix2Interp / DSP::Mix3Interp (mix2interp.cpp, mix3interp.cpp), Wwise 2012.1.

	DISHONORED(port): agent IH4 fork F1 (wave 26). BiquadFilter is the PDB's 36 bytes (coefficients b0 b1 b2 a1 a2 with
	a1 a2 stored negated, memories x1 x2 y1 y2). Deviation named in OnePoleFilter.h (no 4x unroll); the mixers'
	unrolled lanes in retail sum their terms in varying orders (/fp:fast), the port in one order (<= 1 ulp).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

#ifndef AK_RESTRICT
#define AK_RESTRICT __restrict
#endif

namespace DSP
{
	class BiquadFilter
	{
	public:
		enum FilterType
		{
			FilterType_LowShelf		= 0,
			FilterType_Peaking		= 1,
			FilterType_HighShelf	= 2,
			FilterType_LowPass		= 3,
			FilterType_HighPass		= 4,
			FilterType_BandPass		= 5,
			FilterType_Notch		= 6
		};

		struct Coefficients
		{
			AkReal32 fB0;
			AkReal32 fB1;
			AkReal32 fB2;
			AkReal32 fA1;
			AkReal32 fA2;
		};

		struct Memories
		{
			AkReal32 fFFwd1;
			AkReal32 fFFwd2;
			AkReal32 fFFbk1;
			AkReal32 fFFbk2;
		};

		BiquadFilter()
		{
			m_Coefficients.fB0 = 1.f;
			m_Coefficients.fB1 = 0.f;
			m_Coefficients.fB2 = 0.f;
			m_Coefficients.fA1 = 0.f;
			m_Coefficients.fA2 = 0.f;
			Reset();
		}

		void ComputeCoefs( FilterType in_eFilterType, AkUInt32 in_uSampleRate, AkReal32 in_fFreq, AkReal32 in_fGain = 0.f, AkReal32 in_fQ = 1.f );
		void Reset();
		void ProcessBuffer( AkReal32 * io_pBuffer, AkUInt32 in_uNumframes );

		Coefficients	m_Coefficients;
		Memories		m_Memories;
	};

	void Mix2Interp( AkReal32 * AK_RESTRICT in_pfInput1, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT out_pfOutput,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkUInt32 in_uNumFrames );
	void Mix2Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkUInt32 in_uNumFrames );
	void Mix2Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 in_fGain1, AkReal32 in_fGain2, AkUInt32 in_uNumFrames );
	void Mix3Interp( AkReal32 * AK_RESTRICT in_pfInput1, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT in_pfInput3, AkReal32 * AK_RESTRICT out_pfOutput,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkReal32 in_fCurrentGain3, AkReal32 in_fTargetGain3, AkUInt32 in_uNumFrames );
	void Mix3Interp( AkReal32 * AK_RESTRICT io_pfIn1Out, AkReal32 * AK_RESTRICT in_pfInput2, AkReal32 * AK_RESTRICT in_pfInput3,
		AkReal32 in_fCurrentGain1, AkReal32 in_fTargetGain1, AkReal32 in_fCurrentGain2, AkReal32 in_fTargetGain2, AkReal32 in_fCurrentGain3, AkReal32 in_fTargetGain3, AkUInt32 in_uNumFrames );
}

static_assert( sizeof( DSP::BiquadFilter ) == 36, "DSP::BiquadFilter: 2012 PDB sizeof 36" );
