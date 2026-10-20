/*=============================================================================
	AkResamplingPhaseVocoder.cpp - DSP::AkFFTAllButterflies::CAkResamplingPhaseVocoder, Wwise 2012.1
	(plugins/common/akresamplingphasevocoder.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile. Only the harmonizer uses it and the 2013 exe does not link the
	harmonizer, so there are no 2013 twins. Retail's loop guard (at most 100 passes per call, "uMasterBreaker") is kept;
	the interpolation state is written back only after the last channel, as retail.
=============================================================================*/
#include "AkResamplingPhaseVocoder.h"
#include <math.h>

namespace DSP
{
namespace AkFFTAllButterflies
{

/** 2012 0x9921e0 (no 2013 twin): the resampling rings start full of silence. */
void CAkResamplingPhaseVocoder::ResetInputFill()
{
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_ResamplingInputAccumBuf[ i ].Reset();
		m_ResamplingInputAccumBuf[ i ].MarkFull();
	}
}

/** 2012 0x992220 (no 2013 twin). */
AKRESULT CAkResamplingPhaseVocoder::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate, AkUInt32 in_uFFTSize, bool in_bUseInputBuffer )
{
	CAkPhaseVocoder::Init( in_pAllocator, in_uNumChannels, in_uSampleRate, in_uFFTSize, in_bUseInputBuffer );
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		const AKRESULT eResult = m_ResamplingInputAccumBuf[ i ].Init( in_pAllocator, m_uFFTSize + m_uFFTSize / 4 );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	return AK_Success;
}

/** 2012 0x9922a0 (no 2013 twin). */
void CAkResamplingPhaseVocoder::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_ResamplingInputAccumBuf[ i ].Term( in_pAllocator );
	}
	CAkPhaseVocoder::Term( in_pAllocator );
}

/** 2012 0x9922f0 (no 2013 twin). */
void CAkResamplingPhaseVocoder::Reset()
{
	CAkPhaseVocoder::Reset();
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_ResamplingInputAccumBuf[ i ].Reset();
	}
}

/** 2012 0x992330 (no 2013 twin): one channel of resample-then-stretch; returns AK_NoMoreData once the tail is out. */
AKRESULT CAkResamplingPhaseVocoder::ProcessPitchChannel( AkReal32 * in_pfInBuf, AkUInt32 in_uNumFrames, bool in_bNoMoreData, AkUInt32 i, AkReal32 * io_pfOutBuf, AkReal32 in_fResamplingFactor, AkReal32 * in_pfTempStorage )
{
	const AkUInt32 uFFTSize = m_uFFTSize;
	const AkUInt32 uHopSize = uFFTSize / 4;
	AKRESULT eState = AK_DataReady;
	bool bInitPhases = m_bInitPhases;
	AkUInt32 uInputValidFrames = in_uNumFrames;
	AkUInt32 uInputFramesToDiscard = m_uInputFramesToDiscard;
	const AkReal32 fTSFactor = 1.f / in_fResamplingFactor;
	AkUInt32 uOutputValidFrames = 0;
	AkUInt32 uInOffset = 0;
	AkUInt32 uMasterBreaker = 0;
	AkReal32 fInterpPos = m_fInterpPos;
	const AkReal32 fCompGain = (AkReal32)( 1.0 / ( m_TimeWindow.GetCummulativeSum() * 4.0 / (double)uFFTSize ) );
	CAkResamplingCircularBuffer & rInput = m_ResamplingInputAccumBuf[ i ];
	CAkOLACircularBuffer & rOutput = m_OLAOutCircBuf[ i ];
	CAkFreqWindow & rVocoder = m_VocoderWindow[ i ];
	while( ( uInputValidFrames || uOutputValidFrames < in_uNumFrames ) && uMasterBreaker < 100 )
	{
		++uMasterBreaker;
		const AkUInt32 uDiscard = uInputValidFrames >= uInputFramesToDiscard ? uInputFramesToDiscard : uInputValidFrames;
		uInOffset += uDiscard;
		uInputFramesToDiscard -= uDiscard;
		uInputValidFrames -= uDiscard;
		AkUInt32 uConsumed = 0;
		if( in_pfInBuf && uInputValidFrames )
		{
			uConsumed = rInput.PushFrames( &in_pfInBuf[ uInOffset ], uInputValidFrames, in_fResamplingFactor );
		}
		uInOffset += uConsumed;
		uInputValidFrames -= (AkUInt16)uConsumed;

		CAkFreqWindow & rCur = m_FreqWindow[ m_uFreqWindowIndex[ i ] & 1 ][ i ];
		if( !rCur.IsReady() && rInput.ReadFrameBlock( in_pfTempStorage, uFFTSize, in_bNoMoreData ) )
		{
			rInput.AdvanceFrames( uHopSize );
			m_TimeWindow.Apply( in_pfTempStorage, uFFTSize, 1.f );
			rCur.Compute( in_pfTempStorage, uFFTSize, m_pFFTState );
			rCur.CartToPol();
		}
		CAkFreqWindow & rNextAnalysis = m_FreqWindow[ ( m_uFreqWindowIndex[ i ] - 1 ) & 1 ][ i ];
		if( !rNextAnalysis.IsReady() && rInput.ReadFrameBlock( in_pfTempStorage, uFFTSize, in_bNoMoreData ) )
		{
			rInput.AdvanceFrames( uHopSize );
			m_TimeWindow.Apply( in_pfTempStorage, uFFTSize, 1.f );
			rNextAnalysis.Compute( in_pfTempStorage, uFFTSize, m_pFFTState );
			rNextAnalysis.CartToPol();
		}
		{
			CAkFreqWindow & rNext = m_FreqWindow[ ( m_uFreqWindowIndex[ i ] - 1 ) & 1 ][ i ];
			CAkFreqWindow & rPrev = m_FreqWindow[ m_uFreqWindowIndex[ i ] & 1 ][ i ];
			if( rNext.IsReady() && rPrev.IsReady() && !rVocoder.IsReady() )
			{
				rVocoder.ComputeVocoderSpectrum( (AkPolar *)rPrev.Get(), (AkPolar *)rNext.Get(), m_pfPrevSynthesisPhase[ i ], uHopSize, fInterpPos, bInitPhases );
				bInitPhases = false;
			}
		}
		if( rVocoder.IsReady() && rOutput.FramesEmpty() >= uFFTSize )
		{
			rVocoder.ConvertToTimeDomain( in_pfTempStorage, uFFTSize, m_pIFFTState );
			m_TimeWindow.Apply( in_pfTempStorage, uFFTSize, fCompGain );
			rOutput.PushOverlappedWindow( in_pfTempStorage, uHopSize );
			const bool bNoAdvance = ( fInterpPos + fTSFactor ) < 1.f;
			rVocoder.SetReady( false );
			fInterpPos = fInterpPos + fTSFactor;
			if( !bNoAdvance )
			{
				m_FreqWindow[ m_uFreqWindowIndex[ i ] & 1 ][ i ].SetReady( false );
				const double dIntegerPart = floor( fInterpPos );
				const AkUInt32 uFramesAdvance = (AkUInt32)(AkInt64)dIntegerPart;
				if( uFramesAdvance < 2 )
				{
					++m_uFreqWindowIndex[ i ];
				}
				else
				{
					m_FreqWindow[ ( m_uFreqWindowIndex[ i ] - 1 ) & 1 ][ i ].SetReady( false );
					uInputFramesToDiscard = uHopSize * ( uFramesAdvance - 2 ) - rInput.AdvanceFrames( uHopSize * ( uFramesAdvance - 2 ) );
				}
				fInterpPos = fInterpPos - (AkReal32)dIntegerPart;
			}
		}
		const bool bFlushingOLABuffer = in_bNoMoreData && !rInput.FramesReady() && !rVocoder.IsReady() && !rOutput.FramesReady();
		uOutputValidFrames += (AkUInt16)rOutput.PopFrames( &io_pfOutBuf[ uOutputValidFrames ], in_uNumFrames - uOutputValidFrames, bFlushingOLABuffer );
		if( bFlushingOLABuffer && rOutput.IsDoneTail() )
		{
			eState = AK_NoMoreData;
		}
	}
	if( i == m_uNumChannels - 1 )
	{
		m_fInterpPos = fInterpPos;
		m_uInputFramesToDiscard = uInputFramesToDiscard;
		m_bInitPhases = bInitPhases;
	}
	return eState;
}

}
}
