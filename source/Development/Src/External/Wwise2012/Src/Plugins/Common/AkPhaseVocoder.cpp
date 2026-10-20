/*=============================================================================
	AkPhaseVocoder.cpp - DSP::AkFFTAllButterflies::CAkPhaseVocoder, Wwise 2012.1 (plugins/common/akphasevocoder.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. The time-stretch path
	(CAkTimeStretchFX): Hann analysis window (weighted OLA), hop = FFT size / 4, the synthesis spectrum interpolated
	between two analysis frames at m_fInterpPos, which advances by 100 / stretch % per hop. Retail's control flow is kept
	as is, including that a channel which runs out of input jumps to the next channel and that the buffer-level state
	(valid frames, interpolation position, discard count, flags) written back at the end is the last channel's.
=============================================================================*/
#include "AkPhaseVocoder.h"
#include <math.h>
#include <string.h>

namespace DSP
{
namespace AkFFTAllButterflies
{

/** 2012 0x991150, 2013 0x98fb10. */
CAkPhaseVocoder::CAkPhaseVocoder()
	: m_pFFTState( NULL )
	, m_pIFFTState( NULL )
	, m_uNumChannels( 0 )
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		m_pfPrevSynthesisPhase[ i ] = NULL;
	}
}

/** 2012 0x990d80, 2013 0x98f740. */
CAkPhaseVocoder::~CAkPhaseVocoder()
{
}

/** 2012 0x990d90, 2013 0x98f750. */
AKRESULT CAkPhaseVocoder::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate, AkUInt32 in_uFFTSize, bool in_bUseInputBuffer )
{
	m_uFFTSize = in_uFFTSize;
	m_uNumChannels = in_uNumChannels;
	m_uSampleRate = in_uSampleRate;
	ak_fftr_alloc( in_uFFTSize, 0, NULL, (unsigned int *)&m_uFFTSpaceRequirements );
	m_pFFTState = (ak_fftr_state *)AK_PLUGIN_ALLOC( in_pAllocator, m_uFFTSpaceRequirements );
	if( !m_pFFTState )
	{
		return AK_InsufficientMemory;
	}
	ak_fftr_alloc( m_uFFTSize, 1, NULL, (unsigned int *)&m_uIFFTSpaceRequirements );
	m_pIFFTState = (ak_fftr_state *)AK_PLUGIN_ALLOC( in_pAllocator, m_uIFFTSpaceRequirements );
	if( !m_pIFFTState )
	{
		return AK_InsufficientMemory;
	}
	ak_fftr_alloc( m_uFFTSize, 0, m_pFFTState, (unsigned int *)&m_uFFTSpaceRequirements );
	ak_fftr_alloc( m_uFFTSize, 1, m_pIFFTState, (unsigned int *)&m_uIFFTSpaceRequirements );

	AKRESULT eResult = m_TimeWindow.Init( in_pAllocator, m_uFFTSize, CAkTimeWindow::WINDOWTYPE_HANN, true, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		eResult = m_FreqWindow[ 0 ][ i ].Alloc( in_pAllocator, m_uFFTSize );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		eResult = m_FreqWindow[ 1 ][ i ].Alloc( in_pAllocator, m_uFFTSize );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		eResult = m_VocoderWindow[ i ].Alloc( in_pAllocator, m_uFFTSize );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		m_pfPrevSynthesisPhase[ i ] = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( AkReal32 ) * ( m_uFFTSize / 2 + 1 ) );
		if( !m_pfPrevSynthesisPhase[ i ] )
		{
			return AK_InsufficientMemory;
		}
		m_uFreqWindowIndex[ i ] = 0;
	}
	m_bUseInputBuffer = in_bUseInputBuffer;
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		if( in_bUseInputBuffer )
		{
			eResult = m_InputAccumBuf[ i ].Init( in_pAllocator, m_uFFTSize + m_uFFTSize / 4 );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
		eResult = m_OLAOutCircBuf[ i ].Init( in_pAllocator, m_uFFTSize, m_uFFTSize );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	return AK_Success;
}

/** 2012 0x990fb0, 2013 0x98f970. */
void CAkPhaseVocoder::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pFFTState )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pFFTState );
		m_pFFTState = NULL;
	}
	if( m_pIFFTState )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pIFFTState );
		m_pIFFTState = NULL;
	}
	m_TimeWindow.Term( in_pAllocator );
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		m_FreqWindow[ 0 ][ i ].Free( in_pAllocator );
		m_FreqWindow[ 1 ][ i ].Free( in_pAllocator );
		m_VocoderWindow[ i ].Free( in_pAllocator );
		if( m_bUseInputBuffer )
		{
			m_InputAccumBuf[ i ].Term( in_pAllocator );
		}
		m_OLAOutCircBuf[ i ].Term( in_pAllocator );
		if( m_pfPrevSynthesisPhase[ i ] )
		{
			AK_PLUGIN_FREE( in_pAllocator, m_pfPrevSynthesisPhase[ i ] );
			m_pfPrevSynthesisPhase[ i ] = NULL;
		}
	}
}

/** 2012 0x991090, 2013 0x98fa50. */
void CAkPhaseVocoder::Reset()
{
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		m_FreqWindow[ 0 ][ i ].SetReady( false );
		m_FreqWindow[ 1 ][ i ].SetReady( false );
		m_VocoderWindow[ i ].SetReady( false );
		if( m_bUseInputBuffer )
		{
			m_InputAccumBuf[ i ].Reset();
		}
		m_OLAOutCircBuf[ i ].Reset();
		if( m_pfPrevSynthesisPhase[ i ] )
		{
			memset( m_pfPrevSynthesisPhase[ i ], 0, sizeof( AkReal32 ) * ( m_uFFTSize / 2 + 1 ) );
		}
	}
	m_fInterpPos = 0.f;
	m_uInputFramesToDiscard = 0;
	m_bInitPhases = true;
	m_bInputStartFill = true;
}

/** 2012 0x991380, 2013 0x98fd40. */
void CAkPhaseVocoder::Execute( AkAudioBuffer * io_pInBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * io_pOutBuffer, AkReal32 in_fTSFactor, bool in_bEnterNoTSMode, AkReal32 * in_pfTempStorage )
{
	if( in_bEnterNoTSMode )
	{
		m_fInterpPos = 0.f;
		m_bInitPhases = true;
	}
	const AkUInt32 uBlockSize = m_uFFTSize;
	const AkUInt32 uHopSize = uBlockSize / 4;
	const AkReal32 fTimeScale = 100.f / in_fTSFactor;
	const AkReal32 fCompGain = (AkReal32)( 1.0 / ( m_TimeWindow.GetCummulativeSum() * 4.0 / (double)uBlockSize ) );

	AkUInt32 uValidFrames = 0;
	AkUInt32 uOutputValidFrames = 0;
	AkUInt32 uInputFramesToDiscard = 0;
	bool bInitPhases = false;
	bool bInputStartFill = false;
	AkReal32 fInterpPos = 0.f;
	for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
	{
		uOutputValidFrames = io_pOutBuffer->uValidFrames;
		uValidFrames = io_pInBuffer->uValidFrames;
		uInputFramesToDiscard = m_uInputFramesToDiscard;
		bInitPhases = m_bInitPhases;
		AkUInt32 uInOffset = in_uInOffset;
		bInputStartFill = m_bInputStartFill;
		AkReal32 * pInBuf = io_pInBuffer->GetChannel( i );
		fInterpPos = m_fInterpPos;
		AkReal32 * pfOutBuf = io_pOutBuffer->GetChannel( i );
		CAkCircularBuffer & rInput = m_InputAccumBuf[ i ];
		CAkOLACircularBuffer & rOutput = m_OLAOutCircBuf[ i ];
		CAkFreqWindow & rVocoder = m_VocoderWindow[ i ];
		while( 1 )
		{
			const AkUInt32 uDiscard = uValidFrames >= uInputFramesToDiscard ? uInputFramesToDiscard : uValidFrames;
			uInOffset += uDiscard;
			uValidFrames -= uDiscard;
			uInputFramesToDiscard -= uDiscard;
			if( bInputStartFill && !uValidFrames && io_pInBuffer->eState != AK_NoMoreData )
			{
				io_pOutBuffer->eState = AK_DataNeeded;
				goto NextChannel;
			}
			{
				const AkUInt32 uPushed = rInput.PushFrames( &pInBuf[ uInOffset ], uValidFrames );
				if( rInput.FramesEmpty() == 0 )
				{
					bInputStartFill = false;
				}
				uInOffset += uPushed;
				uValidFrames -= (AkUInt16)uPushed;
			}
			bool bNoMoreInputData;
			if( !bInputStartFill || rInput.FramesEmpty() == 0 || uValidFrames )
			{
				bNoMoreInputData = !( io_pInBuffer->eState != AK_NoMoreData || uValidFrames );
			}
			else
			{
				if( io_pInBuffer->eState != AK_NoMoreData )
				{
					io_pOutBuffer->eState = AK_DataNeeded;
					goto NextChannel;
				}
				bNoMoreInputData = true;
			}

			{
				CAkFreqWindow & rCur = m_FreqWindow[ m_uFreqWindowIndex[ i ] & 1 ][ i ];
				if( !rCur.IsReady() && rInput.ReadFrameBlock( in_pfTempStorage, uBlockSize, bNoMoreInputData ) )
				{
					rInput.AdvanceFrames( uHopSize );
					m_TimeWindow.Apply( in_pfTempStorage, uBlockSize, 1.f );
					rCur.Compute( in_pfTempStorage, uBlockSize, m_pFFTState );
					rCur.CartToPol();
				}
				CAkFreqWindow & rNext = m_FreqWindow[ ( m_uFreqWindowIndex[ i ] - 1 ) & 1 ][ i ];
				if( !rNext.IsReady() && rInput.ReadFrameBlock( in_pfTempStorage, uBlockSize, bNoMoreInputData ) )
				{
					rInput.AdvanceFrames( uHopSize );
					m_TimeWindow.Apply( in_pfTempStorage, uBlockSize, 1.f );
					rNext.Compute( in_pfTempStorage, uBlockSize, m_pFFTState );
					rNext.CartToPol();
				}
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
			if( rVocoder.IsReady() && rOutput.FramesEmpty() >= uBlockSize )
			{
				rVocoder.ConvertToTimeDomain( in_pfTempStorage, uBlockSize, m_pIFFTState );
				m_TimeWindow.Apply( in_pfTempStorage, uBlockSize, fCompGain );
				rOutput.PushOverlappedWindow( in_pfTempStorage, uHopSize );
				const bool bNoAdvance = ( fInterpPos + fTimeScale ) < 1.f;
				rVocoder.SetReady( false );
				fInterpPos = fInterpPos + fTimeScale;
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
			{
				const bool bFlushingOLABuffer = bNoMoreInputData && !rInput.FramesReady() && !rVocoder.IsReady() && !rOutput.FramesReady();
				uOutputValidFrames += (AkUInt16)rOutput.PopFrames( &pfOutBuf[ uOutputValidFrames ], io_pOutBuffer->uMaxFrames - uOutputValidFrames, bFlushingOLABuffer );
				if( bFlushingOLABuffer && rOutput.IsDoneTail() )
				{
					io_pOutBuffer->eState = AK_NoMoreData;
					goto NextChannel;
				}
			}
			if( uOutputValidFrames == io_pOutBuffer->uMaxFrames )
			{
				break;
			}
			if( !bNoMoreInputData && !uValidFrames )
			{
				io_pOutBuffer->eState = AK_DataNeeded;
				goto NextChannel;
			}
		}
		io_pOutBuffer->eState = AK_DataReady;
NextChannel:
		;
	}
	io_pInBuffer->uValidFrames = (AkUInt16)uValidFrames;
	io_pOutBuffer->uValidFrames = (AkUInt16)uOutputValidFrames;
	m_fInterpPos = fInterpPos;
	m_uInputFramesToDiscard = uInputFramesToDiscard;
	m_bInitPhases = bInitPhases;
	m_bInputStartFill = bInputStartFill;
}

}
}
