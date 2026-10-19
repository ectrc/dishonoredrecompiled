/*=============================================================================
	AkMixer.cpp - port of akmixer.cpp and akmixersimd.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). Retail's ramp start vectors go through a static scratch array (aVolumes_0);
	here they are built in registers (same values, thread-safe). Kept as retail computes it: MixAndInterleaveStereo and
	MixAndInterleave51 advance their volume ramp by 2 x delta per 4 frames (disassembly 0x957900: mulss by 2.0 then
	shufps), so the final mix reaches only half of a bus volume change within one frame; Mix3D does not set
	uValidFrames for a 4.0 output (0x93ef00).
=============================================================================*/
#include "AkMixer.h"
#include "AkLEngine.h"
#include <math.h>
#include <xmmintrin.h>

/** 2012 0x93ed80; 2013 0x93b700. */
AkSpeakerVolumesStereo3D::AkSpeakerVolumesStereo3D( AkAudioMix & AudioMix, AkReal32 fOneOverNumSamples )
{
	const AkReal32 * pPrev = AudioMix.Previous.aVolumes;
	const AkReal32 * pNext = AudioMix.Next.aVolumes;
	if( CAkLEngine::m_panningRule )
	{
		fLeft = pPrev[ 0 ];
		fLeftDelta = ( pNext[ 0 ] - pPrev[ 0 ] ) * fOneOverNumSamples;
		fRight = pPrev[ 1 ];
		fRightDelta = ( pNext[ 1 ] - pPrev[ 1 ] ) * fOneOverNumSamples;
	}
	else
	{
		const AkReal32 fPrevCenter2 = pPrev[ 2 ] * pPrev[ 2 ];
		fLeft = sqrtf( pPrev[ 0 ] * pPrev[ 0 ] + ( pPrev[ 3 ] * pPrev[ 3 ] + fPrevCenter2 ) * 0.5f );
		fRight = sqrtf( pPrev[ 1 ] * pPrev[ 1 ] + ( fPrevCenter2 + pPrev[ 4 ] * pPrev[ 4 ] ) * 0.5f );
		const AkReal32 fNextCenter2 = pNext[ 2 ] * pNext[ 2 ];
		fLeftDelta = ( sqrtf( pNext[ 0 ] * pNext[ 0 ] + ( pNext[ 3 ] * pNext[ 3 ] + fNextCenter2 ) * 0.5f ) - fLeft ) * fOneOverNumSamples;
		fRightDelta = fOneOverNumSamples * ( sqrtf( 0.5f * ( fNextCenter2 + pNext[ 4 ] * pNext[ 4 ] ) + pNext[ 1 ] * pNext[ 1 ] ) - fRight );
	}
}

/** 2012 0x93ee80; 2013 0x93b800. */
CAkMixer::CAkMixer()
	: m_usMaxFrames( 0 )
	, m_fOneOverNumFrames( 0.f )
{
}

/** 2012 0x93ee90; 2013 0x93b810. */
void CAkMixer::Init( AkUInt16 in_uMaxFrames )
{
	m_usMaxFrames = in_uMaxFrames;
	m_fOneOverNumFrames = 1.f / (AkReal32)in_uMaxFrames;
}

/** 2012 0x93eec0; 2013 0x93b840. */
void CAkMixer::MixStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	MixNStereoPrev( in_pInputBuffer, in_pOutputBuffer );
	in_pOutputBuffer->uValidFrames = m_usMaxFrames;
}

/** 2012 0x93eee0; 2013 0x93b860. */
void CAkMixer::Mix51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	MixN51Prev( in_pInputBuffer, in_pOutputBuffer );
	in_pOutputBuffer->uValidFrames = m_usMaxFrames;
}

/** 2012 0x93ef00; 2013 0x93b880. */
void CAkMixer::Mix3D( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	switch( in_pOutputBuffer->uChannelMask )
	{
	case AK_SPEAKER_SETUP_STEREO:
		MixN3DStereoPrev( in_pInputBuffer, in_pOutputBuffer );
		in_pOutputBuffer->uValidFrames = m_usMaxFrames;
		break;
	case AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT:
		MixN3DFourPrev( in_pInputBuffer, in_pOutputBuffer );
		break;
	case AK_SPEAKER_SETUP_5POINT1:
		MixN3DFiveOnePrev( in_pInputBuffer, in_pOutputBuffer );
		in_pOutputBuffer->uValidFrames = m_usMaxFrames;
		break;
	default:
		in_pOutputBuffer->uValidFrames = m_usMaxFrames;
		break;
	}
}

/** 2012 0x93ef70; 2013 0x93b8f0. */
void CAkMixer::MixFinalStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	MixAndInterleaveStereo( in_pInputBuffer, in_pOutputBuffer );
}

/** 2012 0x93ef80; 2013 0x93b900. */
void CAkMixer::MixFinal51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	MixAndInterleave51( in_pInputBuffer, in_pOutputBuffer );
}

/** 2012 0x9570c0; 2013 0x953960. */
void CAkMixer::AddVolume( AkReal32 * in_pSourceData, AkReal32 * in_pDestData, AkReal32 in_fVolume, AkReal32 in_fVolumeDelta )
{
	const AkReal32 * pSourceEnd = in_pSourceData + m_usMaxFrames;
	if( in_fVolumeDelta == 0.f )
	{
		if( in_fVolume == 0.f )
		{
			return;
		}
		const __m128 vVolume = _mm_set1_ps( in_fVolume );
		do
		{
			_mm_store_ps( in_pDestData, _mm_add_ps( _mm_mul_ps( _mm_load_ps( in_pSourceData ), vVolume ), _mm_load_ps( in_pDestData ) ) );
			_mm_store_ps( in_pDestData + 4, _mm_add_ps( _mm_mul_ps( _mm_load_ps( in_pSourceData + 4 ), vVolume ), _mm_load_ps( in_pDestData + 4 ) ) );
			in_pSourceData += 8;
			in_pDestData += 8;
		}
		while( in_pSourceData < pSourceEnd );
		return;
	}
	__m128 vVolume0 = _mm_setr_ps( in_fVolume, in_fVolume + in_fVolumeDelta, in_fVolumeDelta * 2.f + in_fVolume, in_fVolumeDelta * 3.f + in_fVolume );
	const __m128 vStep4 = _mm_set1_ps( in_fVolumeDelta * 4.f );
	__m128 vVolume1 = _mm_add_ps( vVolume0, vStep4 );
	const __m128 vStep8 = _mm_add_ps( vStep4, vStep4 );
	do
	{
		_mm_store_ps( in_pDestData, _mm_add_ps( _mm_mul_ps( _mm_load_ps( in_pSourceData ), vVolume0 ), _mm_load_ps( in_pDestData ) ) );
		_mm_store_ps( in_pDestData + 4, _mm_add_ps( _mm_mul_ps( _mm_load_ps( in_pSourceData + 4 ), vVolume1 ), _mm_load_ps( in_pDestData + 4 ) ) );
		in_pSourceData += 8;
		in_pDestData += 8;
		vVolume0 = _mm_add_ps( vVolume0, vStep8 );
		vVolume1 = _mm_add_ps( vVolume1, vStep8 );
	}
	while( in_pSourceData < pSourceEnd );
}

/** 2012 0x957220; 2013 0x953ac0. */
void CAkMixer::MixNStereoPrev( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	const AkReal32 * pPrev = in_pInputBuffer->AudioMix.Previous.aVolumes;
	const AkReal32 * pNext = in_pInputBuffer->AudioMix.Next.aVolumes;
	for( AkUInt32 uSpeaker = 0; uSpeaker < 2; ++uSpeaker )
	{
		AddVolume( in_pInputBuffer->GetChannel( uSpeaker ), in_pOutputBuffer->GetChannel( uSpeaker ), pPrev[ uSpeaker ], ( pNext[ uSpeaker ] - pPrev[ uSpeaker ] ) * m_fOneOverNumFrames );
	}
}

/** 2012 0x9572d0; 2013 0x953b70. FL FR C, then LFE (plane 5), RL, RR, each plane with its own speaker's volume. */
void CAkMixer::MixN51Prev( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	static const AkUInt32 s_uOrder[ 6 ] = { 0, 1, 2, 5, 3, 4 };
	const AkReal32 * pPrev = in_pInputBuffer->AudioMix.Previous.aVolumes;
	const AkReal32 * pNext = in_pInputBuffer->AudioMix.Next.aVolumes;
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		const AkUInt32 uSpeaker = s_uOrder[ i ];
		AddVolume( in_pInputBuffer->GetChannel( uSpeaker ), in_pOutputBuffer->GetChannel( uSpeaker ), pPrev[ uSpeaker ], ( pNext[ uSpeaker ] - pPrev[ uSpeaker ] ) * m_fOneOverNumFrames );
	}
}

/** 2012 0x9574b0; 2013 0x953d50. Every input plane is added into all six output planes with its own mix. */
void CAkMixer::MixN3DFiveOnePrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	static const AkUInt32 s_uOrder[ 6 ] = { 0, 1, 2, 5, 3, 4 };
	const AkUInt32 uNumChannels = in_pInputBuffer->NumChannels();
	for( AkUInt32 uChannel = 0; uChannel < uNumChannels; ++uChannel )
	{
		const AkReal32 * pPrev = in_pInputBuffer->AudioMix[ uChannel ].Previous.aVolumes;
		const AkReal32 * pNext = in_pInputBuffer->AudioMix[ uChannel ].Next.aVolumes;
		AkReal32 * pSource = in_pInputBuffer->GetChannel( uChannel );
		for( AkUInt32 i = 0; i < 6; ++i )
		{
			const AkUInt32 uSpeaker = s_uOrder[ i ];
			AddVolume( pSource, in_pOutputBuffer->GetChannel( uSpeaker ), pPrev[ uSpeaker ], ( pNext[ uSpeaker ] - pPrev[ uSpeaker ] ) * m_fOneOverNumFrames );
		}
	}
}

/** 2012 0x9576c0; 2013 0x953f60. 4.0 output: FL, FR, RL (volume 3) into plane 2, RR (volume 4) into plane 3. */
void CAkMixer::MixN3DFourPrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	static const AkUInt32 s_uSpeaker[ 4 ] = { 0, 1, 3, 4 };
	const AkUInt32 uNumChannels = in_pInputBuffer->NumChannels();
	for( AkUInt32 uChannel = 0; uChannel < uNumChannels; ++uChannel )
	{
		const AkReal32 * pPrev = in_pInputBuffer->AudioMix[ uChannel ].Previous.aVolumes;
		const AkReal32 * pNext = in_pInputBuffer->AudioMix[ uChannel ].Next.aVolumes;
		AkReal32 * pSource = in_pInputBuffer->GetChannel( uChannel );
		for( AkUInt32 uOut = 0; uOut < 4; ++uOut )
		{
			const AkUInt32 uSpeaker = s_uSpeaker[ uOut ];
			AddVolume( pSource, in_pOutputBuffer->GetChannel( uOut ), pPrev[ uSpeaker ], ( pNext[ uSpeaker ] - pPrev[ uSpeaker ] ) * m_fOneOverNumFrames );
		}
	}
}

/** 2012 0x957840; 2013 0x9540e0. */
void CAkMixer::MixN3DStereoPrev( AkAudioBufferMix * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	const AkUInt32 uNumChannels = in_pInputBuffer->NumChannels();
	for( AkUInt32 uChannel = 0; uChannel < uNumChannels; ++uChannel )
	{
		AkReal32 * pSource = in_pInputBuffer->GetChannel( uChannel );
		AkSpeakerVolumesStereo3D Volumes( in_pInputBuffer->AudioMix[ uChannel ], m_fOneOverNumFrames );
		AddVolume( pSource, in_pOutputBuffer->GetChannel( 0 ), Volumes.fLeft, Volumes.fLeftDelta );
		AddVolume( pSource, in_pOutputBuffer->GetChannel( 1 ), Volumes.fRight, Volumes.fRightDelta );
	}
}

/** 2012 0x957900; 2013 0x9541a0. */
void CAkMixer::MixAndInterleaveStereo( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	const AkReal32 * pPrev = in_pInputBuffer->AudioMix.Previous.aVolumes;
	const AkReal32 * pNext = in_pInputBuffer->AudioMix.Next.aVolumes;
	const AkReal32 fDeltaL = ( pNext[ 0 ] - pPrev[ 0 ] ) * m_fOneOverNumFrames;
	const AkReal32 fDeltaR = ( pNext[ 1 ] - pPrev[ 1 ] ) * m_fOneOverNumFrames;
	__m128 vVolumeL = _mm_setr_ps( pPrev[ 0 ], pPrev[ 0 ] + fDeltaL, fDeltaL * 2.f + pPrev[ 0 ], fDeltaL * 3.f + pPrev[ 0 ] );
	__m128 vVolumeR = _mm_setr_ps( pPrev[ 1 ], pPrev[ 1 ] + fDeltaR, fDeltaR * 2.f + pPrev[ 1 ], fDeltaR * 3.f + pPrev[ 1 ] );
	const __m128 vStepL = _mm_set1_ps( fDeltaL * 2.f );
	const __m128 vStepR = _mm_set1_ps( fDeltaR * 2.f );
	const __m128 * pInL = (const __m128 *)in_pInputBuffer->GetChannel( 0 );
	const __m128 * pInR = (const __m128 *)in_pInputBuffer->GetChannel( 1 );
	__m128 * pOut = (__m128 *)in_pOutputBuffer->pData;
	for( AkUInt32 n = m_usMaxFrames >> 2; n; --n )
	{
		const __m128 vL = _mm_mul_ps( *pInL++, vVolumeL );
		vVolumeL = _mm_add_ps( vStepL, vVolumeL );
		const __m128 vR = _mm_mul_ps( *pInR++, vVolumeR );
		vVolumeR = _mm_add_ps( vStepR, vVolumeR );
		*pOut++ = _mm_unpacklo_ps( vL, vR );
		*pOut++ = _mm_unpackhi_ps( vL, vR );
	}
}

/** 2012 0x957a40; 2013 0x9542e0. Output interleaved in WAV order FL FR C LFE RL RR. */
void CAkMixer::MixAndInterleave51( AkAudioBufferBus * in_pInputBuffer, AkPipelineBufferBase * in_pOutputBuffer )
{
	const AkReal32 * pPrev = in_pInputBuffer->AudioMix.Previous.aVolumes;
	const AkReal32 * pNext = in_pInputBuffer->AudioMix.Next.aVolumes;
	__m128 vVolume[ 6 ];
	__m128 vStep[ 6 ];
	for( AkUInt32 uSpeaker = 0; uSpeaker < 6; ++uSpeaker )
	{
		const AkReal32 fPrev = pPrev[ uSpeaker ];
		const AkReal32 fDelta = ( pNext[ uSpeaker ] - fPrev ) * m_fOneOverNumFrames;
		vVolume[ uSpeaker ] = _mm_setr_ps( fPrev, fDelta + fPrev, fDelta * 2.f + fPrev, fDelta * 3.f + fPrev );
		vStep[ uSpeaker ] = _mm_set1_ps( fDelta * 2.f );
	}
	const __m128 * pIn[ 6 ];
	for( AkUInt32 uSpeaker = 0; uSpeaker < 6; ++uSpeaker )
	{
		pIn[ uSpeaker ] = (const __m128 *)in_pInputBuffer->GetChannel( uSpeaker );
	}
	__m128 * pOut = (__m128 *)in_pOutputBuffer->pData;
	for( AkUInt32 n = m_usMaxFrames >> 2; n; --n )
	{
		__m128 vMixed[ 6 ];
		for( AkUInt32 uSpeaker = 0; uSpeaker < 6; ++uSpeaker )
		{
			vMixed[ uSpeaker ] = _mm_mul_ps( *pIn[ uSpeaker ]++, vVolume[ uSpeaker ] );
			vVolume[ uSpeaker ] = _mm_add_ps( vStep[ uSpeaker ], vVolume[ uSpeaker ] );
		}
		const __m128 vFL = vMixed[ 0 ];
		const __m128 vFR = vMixed[ 1 ];
		const __m128 vC = vMixed[ 2 ];
		const __m128 vRL = vMixed[ 3 ];
		const __m128 vRR = vMixed[ 4 ];
		const __m128 vLFE = vMixed[ 5 ];
		const __m128 vCLfeLo = _mm_shuffle_ps( vC, vLFE, 0x44 );
		const __m128 vCLfeHi = _mm_shuffle_ps( vC, vLFE, 0xEE );
		const __m128 vFrontLo = _mm_shuffle_ps( vFL, vFR, 0x44 );
		const __m128 vFrontHi = _mm_shuffle_ps( vFL, vFR, 0xEE );
		const __m128 vRearHi = _mm_shuffle_ps( vRL, vRR, 0xEE );
		const __m128 vRearLo = _mm_shuffle_ps( vRL, vRR, 0x44 );
		pOut[ 0 ] = _mm_shuffle_ps( vFrontLo, vCLfeLo, 0x88 );
		pOut[ 1 ] = _mm_shuffle_ps( vRearLo, vFrontLo, 0xD8 );
		pOut[ 2 ] = _mm_shuffle_ps( vCLfeLo, vRearLo, 0xDD );
		pOut[ 3 ] = _mm_shuffle_ps( vFrontHi, vCLfeHi, 0x88 );
		pOut[ 4 ] = _mm_shuffle_ps( vRearHi, vFrontHi, 0xD8 );
		pOut[ 5 ] = _mm_shuffle_ps( vCLfeHi, vRearHi, 0xDD );
		pOut += 6;
	}
}
