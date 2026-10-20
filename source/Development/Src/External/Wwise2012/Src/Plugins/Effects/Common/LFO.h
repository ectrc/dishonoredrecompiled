/*=============================================================================
	LFO.h - DSP::LFO (Wwise 2012.1, plugins/effects/common/lfo.h): waveforms, parameters, the per-oscillator state and
	the multichannel phase setup. Agent IH4 (fork F2). Layouts are the 2012 PDB's (Params 16, PhaseParams 12,
	AllParams 28, State 24 bytes). The phase is in radians for the sine and in [0, 1) for the other waveforms.
=============================================================================*/
#ifndef _AK_LFO_H_
#define _AK_LFO_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include "OnePoleFilter.h"

namespace DSP
{
	namespace LFO
	{
		enum Waveform
		{
			WAVEFORM_SINE = 0,
			WAVEFORM_TRIANGLE,
			WAVEFORM_SQUARE,
			WAVEFORM_SAW_UP,
			WAVEFORM_SAW_DOWN
		};

		struct Params
		{
			Waveform	eWaveform;
			AkReal32	fFrequency;
			AkReal32	fSmooth;
			AkReal32	fPWM;
		};

		struct State
		{
			OnePoleFilter	filter;
			AkReal32		fPhase;
			AkReal32		fPhaseDelta;
			Waveform		eWaveform;
		};

		namespace MultiChannel
		{
			enum PhaseMode
			{
				PHASE_MODE_LEFT_RIGHT = 0,
				PHASE_MODE_FRONT_REAR,
				PHASE_MODE_CIRCULAR,
				PHASE_MODE_RANDOM
			};

			struct PhaseParams
			{
				AkReal32	fPhaseOffset;
				AkReal32	fPhaseSpread;
				PhaseMode	ePhaseMode;
			};

			struct AllParams
			{
				Params		lfoParams;
				PhaseParams	phaseParams;
			};

			void ComputeInitialPhase( AkUInt32 in_uChannelMask, const PhaseParams & in_phaseParams, AkReal32 * const out_uChannelPhase );
		}
	}
}

static_assert( sizeof( DSP::LFO::Params ) == 16, "DSP::LFO::Params: 2012 PDB sizeof 16" );
static_assert( sizeof( DSP::LFO::State ) == 24, "DSP::LFO::State: 2012 PDB sizeof 24" );
static_assert( sizeof( DSP::LFO::MultiChannel::PhaseParams ) == 12, "DSP::LFO::MultiChannel::PhaseParams: 2012 PDB sizeof 12" );
static_assert( sizeof( DSP::LFO::MultiChannel::AllParams ) == 28, "DSP::LFO::MultiChannel::AllParams: 2012 PDB sizeof 28" );

#endif
