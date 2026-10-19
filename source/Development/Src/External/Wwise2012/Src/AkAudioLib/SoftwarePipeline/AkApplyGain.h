/*=============================================================================
	AkApplyGain.h - AK::DSP::ApplyGain / ApplyGainRamp (sdk headers akapplygain.h): constant or linearly ramped gain on
	one channel or on every channel of a buffer (optionally skipping the LFE, which is the last channel).

	DISHONORED(port): agent IH2 (wave 26). Header-inline as in the SDK; retail instantiates them per module (2012
	ApplyGainRamp 0x97ae90, ApplyGain( float *, gain ) 0x97b000, ApplyGain( buffer, cur, target, LFE ) 0x97b930,
	ApplyGain( float *, cur, target ) 0x992cd0 and their copies in every effect). The SSE body is 4 frames per step with
	the ramp step computed over the frames that are a multiple of 4; the scalar tail recomputes the step over all frames
	(retail's own arithmetic, kept).
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"
#include <xmmintrin.h>

namespace AK
{
	namespace DSP
	{
		inline void ApplyGainRamp( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames, AkReal32 in_fCurGain, AkReal32 in_fTargetGain )
		{
			const AkReal32 fDiff = in_fTargetGain - in_fCurGain;
			AkReal32 * pfEnd = io_pfBuffer + in_uNumFrames;
			AkReal32 * pfVectorEnd = io_pfBuffer + ( in_uNumFrames & ~3 );
			const AkReal32 fVectorStep = fDiff / (AkReal32)( (AkUInt32)( 4 * ( in_uNumFrames >> 2 ) ) );
			AkReal32 fGain = in_fCurGain;
			__m128 vGain = _mm_setr_ps( fGain, fVectorStep + fGain, ( fVectorStep + fGain ) + fVectorStep, ( ( fVectorStep + fGain ) + fVectorStep ) + fVectorStep );
			const __m128 vStep = _mm_set1_ps( fVectorStep * 4.f );
			while( io_pfBuffer < pfVectorEnd )
			{
				_mm_store_ps( io_pfBuffer, _mm_mul_ps( _mm_load_ps( io_pfBuffer ), vGain ) );
				io_pfBuffer += 4;
				vGain = _mm_add_ps( vStep, vGain );
			}
			if( io_pfBuffer < pfEnd )
			{
				const AkReal32 fStep = fDiff / (AkReal32)in_uNumFrames;
				while( io_pfBuffer < pfEnd )
				{
					*io_pfBuffer++ *= fGain;
					fGain += fStep;
				}
			}
		}

		inline void ApplyGain( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames, AkReal32 in_fGain )
		{
			if( in_fGain == 1.f )
			{
				return;
			}
			AkReal32 * pfEnd = io_pfBuffer + in_uNumFrames;
			AkReal32 * pfVectorEnd = io_pfBuffer + ( in_uNumFrames & ~3 );
			const __m128 vGain = _mm_set1_ps( in_fGain );
			while( io_pfBuffer < pfVectorEnd )
			{
				_mm_store_ps( io_pfBuffer, _mm_mul_ps( _mm_load_ps( io_pfBuffer ), vGain ) );
				io_pfBuffer += 4;
			}
			while( io_pfBuffer < pfEnd )
			{
				*io_pfBuffer++ *= in_fGain;
			}
		}

		inline void ApplyGain( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames, AkReal32 in_fCurGain, AkReal32 in_fTargetGain )
		{
			if( in_fTargetGain == in_fCurGain )
			{
				ApplyGain( io_pfBuffer, in_uNumFrames, in_fCurGain );
			}
			else
			{
				ApplyGainRamp( io_pfBuffer, in_uNumFrames, in_fCurGain, in_fTargetGain );
			}
		}

		inline void ApplyGain( AkAudioBuffer * io_pBuffer, AkReal32 in_fCurGain, AkReal32 in_fTargetGain, bool in_bProcessLFE = true )
		{
			AkUInt32 uNumChannels = io_pBuffer->NumChannels();
			if( !in_bProcessLFE && ( io_pBuffer->uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) )
			{
				--uNumChannels;
			}
			const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
			if( in_fTargetGain == in_fCurGain )
			{
				for( AkUInt32 i = 0; i < uNumChannels; ++i )
				{
					ApplyGain( io_pBuffer->GetChannel( i ), uNumFrames, in_fCurGain );
				}
			}
			else
			{
				for( AkUInt32 i = 0; i < uNumChannels; ++i )
				{
					ApplyGainRamp( io_pBuffer->GetChannel( i ), uNumFrames, in_fCurGain, in_fTargetGain );
				}
			}
		}
	}
}
