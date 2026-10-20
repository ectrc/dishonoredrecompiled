/*=============================================================================
	AkDelayFXDSP.cpp - CAkDelayFXDSP, Wwise 2012.1 (samples/plugins/akdelay/sources/audioenginefx/akdelayfxdsp.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. A feedback comb per channel:
	out = (wet * d + (1 - wet) * in) * level, d' = feedback * d + in, with the delay length rounded up to 4 frames and
	wet / feedback / level ramped per sample from the previous buffer's values, four frames per SSE register as retail
	(only whole groups of four frames are processed; the engine's buffers always are). In send mode the mix is fully wet
	and an unprocessed LFE is silenced.
	Deviation (named): retail loads and stores the buffer and the delay memory with aligned SSE moves; unaligned moves
	are used here (same values; the plug-in allocator's alignment is the lower engine's choice).
=============================================================================*/
#include "AkDelayFXDSP.h"
#include <math.h>
#include <string.h>
#include <xmmintrin.h>

/** 2012 0x97cb20, 2013 0x979320. */
CAkDelayFXDSP::CAkDelayFXDSP()
	: m_uNumProcessedChannels( 0 )
{
	m_FXTailHandler.uTailFramesRemaining = (AkUInt32)-1;
	m_FXTailHandler.uTotalTailFrames = 0;
}

/** 2012 0x97c430, 2013 0x978c30. */
void CAkDelayFXDSP::Setup( AkDelayFXParams * pInitialParams, bool in_bIsSendMode, AkUInt32 in_uSampleRate )
{
	m_bSendMode = in_bIsSendMode;
	m_PreviousParams = *pInitialParams;
	m_uSampleRate = in_uSampleRate;
	if( !m_PreviousParams.RTPC.bFeedbackEnabled )
	{
		m_PreviousParams.RTPC.fFeedback = 0.f;
	}
	if( in_bIsSendMode )
	{
		m_PreviousParams.RTPC.fWetDryMix = 1.f;
	}
}

/** 2012 0x97c6b0, 2013 0x978eb0. */
AKRESULT CAkDelayFXDSP::InitDelay( AK::IAkPluginMemAlloc * in_pAllocator, AkDelayFXParams * pParams, AkChannelMask in_uChannelMask )
{
	m_DelayMem.Term( in_pAllocator );
	m_uNumProcessedChannels = AkGetNumChannels( in_uChannelMask );
	m_bProcessLFE = pParams->NonRTPC.bProcessLFE;
	if( ( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) && !m_bProcessLFE )
	{
		--m_uNumProcessedChannels;
	}
	return m_DelayMem.Init( in_pAllocator, (AkUInt32)(AkInt64)( (double)m_uSampleRate * pParams->NonRTPC.fDelayTime ), m_uNumProcessedChannels );
}

/** 2012 0x97c740, 2013 0x978f40. */
void CAkDelayFXDSP::ResetDelay()
{
	m_DelayMem.Reset();
}

/** 2012 0x97c750, 2013 0x978f50. */
void CAkDelayFXDSP::TermDelay( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_DelayMem.Term( in_pAllocator );
}

/** 2012 0x97c760, 2013 0x978f60: the tail lasts until the echoes fall 60 dB (100 echoes at most). */
void CAkDelayFXDSP::ComputeTailLength( bool in_bFeedbackEnabled, AkReal32 in_fFeedbackValue )
{
	if( !in_bFeedbackEnabled || in_fFeedbackValue == 0.f )
	{
		m_uTailLength = m_DelayMem.GetDelayLength();
	}
	else
	{
		const double dIterationGain = log10( (double)in_fFeedbackValue ) * 20.0;
		AkReal32 fNumIterations;
		if( dIterationGain >= -0.60000002f )
		{
			fNumIterations = 100.f;
		}
		else
		{
			const AkReal32 fIterationGain = (AkReal32)dIterationGain;
			fNumIterations = -60.f / fIterationGain;
		}
		m_uTailLength = (AkUInt32)(AkInt64)( (double)m_DelayMem.GetDelayLength() * fNumIterations );
	}
}

/** 2012 0x97c810, 2013 0x979010. */
void CAkDelayFXDSP::Process( AkAudioBuffer * io_pBuffer, AkDelayFXParams * pCurrentParams )
{
	m_FXTailHandler.HandleTail( io_pBuffer, m_uTailLength );
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	if( !pCurrentParams->RTPC.bFeedbackEnabled )
	{
		pCurrentParams->RTPC.fFeedback = 0.f;
	}
	if( m_bSendMode )
	{
		pCurrentParams->RTPC.fWetDryMix = 1.f;
	}
	AkUInt32 uOffset = 0;
	if( m_uNumProcessedChannels )
	{
		const AkReal32 fNumFrames = (AkReal32)uNumFrames;
		const AkUInt32 uDelayLength = m_DelayMem.GetDelayLength();

		const AkReal32 fLevelStart = m_PreviousParams.RTPC.fOutputLevel;
		const AkReal32 fLevelInc = ( pCurrentParams->RTPC.fOutputLevel - fLevelStart ) / fNumFrames;
		const __m128 vLevelInc = _mm_set1_ps( fLevelInc * 4.f );
		const AkReal32 fLevel1 = fLevelInc + fLevelStart;
		const __m128 vLevelStart = _mm_setr_ps( fLevelStart, fLevel1, fLevel1 + fLevelInc, ( fLevel1 + fLevelInc ) + fLevelInc );

		const AkReal32 fFeedbackStart = m_PreviousParams.RTPC.fFeedback;
		const AkReal32 fFeedbackInc = ( pCurrentParams->RTPC.fFeedback - fFeedbackStart ) / fNumFrames;
		const __m128 vFeedbackInc = _mm_set1_ps( fFeedbackInc * 4.f );
		const AkReal32 fFeedback1 = fFeedbackInc + fFeedbackStart;
		const __m128 vFeedbackStart = _mm_setr_ps( fFeedbackStart, fFeedback1, fFeedback1 + fFeedbackInc, ( fFeedback1 + fFeedbackInc ) + fFeedbackInc );

		const AkReal32 fWetStart = m_PreviousParams.RTPC.fWetDryMix;
		const AkReal32 fWetInc = ( pCurrentParams->RTPC.fWetDryMix - fWetStart ) / fNumFrames;
		const __m128 vWetInc = _mm_set1_ps( fWetInc * 4.f );
		const AkReal32 fWet1 = fWetInc + fWetStart;
		const AkReal32 fWet2 = fWet1 + fWetInc;
		const __m128 vWetStart = _mm_setr_ps( fWetStart, fWet1, fWet2, fWet2 + fWetInc );
		const __m128 vOne = _mm_set1_ps( 1.f );

		const AkUInt32 uStartOffset = m_DelayMem.GetCurrentOffset();
		for( AkUInt32 uChannel = 0; uChannel < m_uNumProcessedChannels; ++uChannel )
		{
			AkReal32 * pfBuf = io_pBuffer->GetChannel( uChannel );
			AkReal32 * pfDelay = m_DelayMem.GetChannel( uChannel );
			__m128 vWet = vWetStart;
			__m128 vLevel = vLevelStart;
			__m128 vFeedback = vFeedbackStart;
			__m128 vDry = _mm_sub_ps( vOne, vWetStart );
			uOffset = uStartOffset;
			AkUInt32 uFramesProcessed = 0;
			while( uFramesProcessed < uNumFrames )
			{
				AkUInt32 uFramesBeforeWrap = uDelayLength - uOffset;
				if( uFramesBeforeWrap >= uNumFrames - uFramesProcessed )
				{
					uFramesBeforeWrap = uNumFrames - uFramesProcessed;
				}
				AkReal32 * pfDelayCur = pfDelay + uOffset;
				for( AkUInt32 uVectors = uFramesBeforeWrap >> 2; uVectors; --uVectors )
				{
					const __m128 vIn = _mm_loadu_ps( pfBuf );
					const __m128 vDelay = _mm_loadu_ps( pfDelayCur );
					const __m128 vOut = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( vWet, vDelay ), _mm_mul_ps( vIn, vDry ) ), vLevel );
					_mm_storeu_ps( pfDelayCur, _mm_add_ps( _mm_mul_ps( vFeedback, vDelay ), vIn ) );
					_mm_storeu_ps( pfBuf, vOut );
					vFeedback = _mm_add_ps( vFeedback, vFeedbackInc );
					vLevel = _mm_add_ps( vLevel, vLevelInc );
					vWet = _mm_add_ps( vWetInc, vWet );
					vDry = _mm_sub_ps( vOne, vWet );
					pfBuf += 4;
					pfDelayCur += 4;
				}
				uOffset += uFramesBeforeWrap;
				uFramesProcessed += uFramesBeforeWrap;
				if( uOffset == uDelayLength )
				{
					uOffset = 0;
				}
			}
		}
	}
	m_DelayMem.SetCurrentOffset( uOffset );
	const AkChannelMask uChannelMask = io_pBuffer->uChannelMask;
	if( uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
	{
		AkReal32 * pfLFE = io_pBuffer->GetChannel( AkGetNumChannels( uChannelMask ) - 1 );
		if( pfLFE && m_bSendMode && !pCurrentParams->NonRTPC.bProcessLFE )
		{
			memset( pfLFE, 0, sizeof( AkReal32 ) * uNumFrames );
		}
	}
	m_PreviousParams = *pCurrentParams;
}
