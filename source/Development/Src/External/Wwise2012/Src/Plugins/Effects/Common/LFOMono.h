/*=============================================================================
	LFOMono.h - DSP::MonoLFO<POLARITY> and the DSP::Unipolar polarity (Wwise 2012.1, plugins/effects/common/lfomono.h).
	Agent IH4 (fork F2). MonoLFO<Unipolar> is 24 bytes (2012 PDB: the empty polarity base + LFO::State). Retail
	instantiates it only for the tremolo: SetParams 2012 0x99ae40 (2013 0x993b40), ComputeFilterCoefsFromParams
	0x99aef0 (0x993bf0), Setup 0x99b000 (0x993d00). The per-sample generation is inlined into CAkTremoloFX::Execute.
=============================================================================*/
#ifndef _AK_LFOMONO_H_
#define _AK_LFOMONO_H_

#include "LFO.h"
#include <math.h>

namespace DSP
{
	/** The unipolar output range [0, 1] (an empty policy class in the PDB, sizeof 1). */
	class Unipolar
	{
	};

	template < class POLARITY >
	class MonoLFO : public POLARITY
	{
	public:
		/** 2012 0x99aef0, 2013 0x993bf0: the smoothing low-pass sits between the LFO rate (smooth 1) and Nyquist (smooth 0). */
		static void ComputeFilterCoefsFromParams( AkUInt32 in_uSampleRate, const LFO::Params & in_lfoParams, AkReal32 & out_fB0, AkReal32 & out_fA1 )
		{
			const AkReal32 fSmooth = in_lfoParams.fSmooth;
			if( fSmooth == 0.f )
			{
				OnePoleFilter::ComputeCoefs( OnePoleFilter::FILTERCURVETYPE_NONE, 0.f, 0, out_fB0, out_fA1 );
			}
			else
			{
				const double dNyquist = (double)in_uSampleRate * 0.5;
				const double dExponent = -( log( dNyquist / in_lfoParams.fFrequency ) * fSmooth ) * 1.442695040888963407;
				const AkReal32 fFc = (AkReal32)( dNyquist * pow( 2.0, dExponent ) );
				OnePoleFilter::ComputeCoefs( OnePoleFilter::FILTERCURVETYPE_LOWPASS, fFc, in_uSampleRate, out_fB0, out_fA1 );
			}
		}

		/** 2012 0x99ae40, 2013 0x993b40: a waveform change between sine and the others rescales the phase. */
		void SetParams( AkUInt32 in_uSampleRate, const LFO::Params & in_lfoParams, AkReal32 in_fB0, AkReal32 in_fA1 )
		{
			const AkReal32 fPhaseDelta = (AkReal32)( in_lfoParams.fFrequency / (double)in_uSampleRate );
			m_state.fPhaseDelta = fPhaseDelta;
			if( in_lfoParams.eWaveform == LFO::WAVEFORM_SINE )
			{
				m_state.fPhaseDelta = fPhaseDelta * 6.2831855f;
			}
			const LFO::Waveform eOldWaveform = m_state.eWaveform;
			m_state.filter.fB0 = in_fB0;
			m_state.filter.fA1 = in_fA1;
			if( eOldWaveform != in_lfoParams.eWaveform )
			{
				if( eOldWaveform == LFO::WAVEFORM_SINE )
				{
					m_state.fPhase = m_state.fPhase * 0.15915494f;
				}
				else if( in_lfoParams.eWaveform == LFO::WAVEFORM_SINE )
				{
					m_state.fPhase = m_state.fPhase * 6.2831855f;
				}
			}
			m_state.eWaveform = in_lfoParams.eWaveform;
		}

		/** 2012 0x99b000, 2013 0x993d00: in_fInitPhase is in degrees. */
		void Setup( AkUInt32 in_uSampleRate, const LFO::Params & in_lfoParams, AkReal32 in_fInitPhase )
		{
			AkReal32 fB0, fA1;
			ComputeFilterCoefsFromParams( in_uSampleRate, in_lfoParams, fB0, fA1 );
			SetParams( in_uSampleRate, in_lfoParams, fB0, fA1 );
			const AkReal32 fPhaseRange = in_lfoParams.eWaveform != LFO::WAVEFORM_SINE ? 1.f : 6.2831855f;
			AkReal32 fPhase = ( fPhaseRange * in_fInitPhase ) * 0.0027777778f;
			if( m_state.eWaveform == LFO::WAVEFORM_TRIANGLE )
			{
				fPhase = fPhase + 0.25f;
			}
			else if( m_state.eWaveform == LFO::WAVEFORM_SAW_UP )
			{
				fPhase = fPhase + 0.5f;
			}
			if( fPhase < 0.f )
			{
				fPhase = fPhase + fPhaseRange;
			}
			if( fPhase >= fPhaseRange )
			{
				fPhase = fPhase - fPhaseRange;
			}
			m_state.fPhase = fPhase;
			m_state.fPhase = (AkReal32)fmod( (double)fPhase, m_state.eWaveform != LFO::WAVEFORM_SINE ? 1.0 : 6.283185482025146 );
		}

		LFO::State & GetState() { return m_state; }

	protected:
		LFO::State m_state;
	};
}

static_assert( sizeof( DSP::MonoLFO<DSP::Unipolar> ) == 24, "DSP::MonoLFO<DSP::Unipolar>: 2012 PDB sizeof 24" );

#endif
