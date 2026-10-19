/*=============================================================================
	AkSrcBase.cpp - CAkSrcBaseEx (aksrcbase.cpp, 2012 0x95ec40..0x95efa0 and the akvplsrcnode.h inline OnLoopComplete
	0x955940; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26).
=============================================================================*/
#include "AkSrcBase.h"
#include "AkCommon.h"
#include "AkPositionRepository.h"
#include "AkAudioLibGlobals.h"

/** 2012 0x95efa0. */
CAkSrcBaseEx::CAkSrcBaseEx( CAkPBI * in_pCtx )
	: CAkVPLSrcNode( in_pCtx )
	, m_uTotalSamples( 0 )
	, m_uCurSample( 0 )
	, m_uDataSize( 0 )
	, m_uDataOffset( 0 )
	, m_uPCMLoopStart( 0 )
	, m_uPCMLoopEnd( 0 )
{
	m_uLoopCnt = m_pCtx->m_LoopCount;
}

/** 2012 0x95ed40. */
CAkSrcBaseEx::~CAkSrcBaseEx()
{
}

/** 2012 0x95ec40. */
void CAkSrcBaseEx::StopStream()
{
	m_markers.Free();
}

/** 2012 0x95ec50. */
AKRESULT CAkSrcBaseEx::StopLooping()
{
	m_uLoopCnt = 1;
	return AK_Success;
}

/** 2012 0x95ec60: ms over every loop; 0 for an infinite loop. */
AkReal32 CAkSrcBaseEx::GetDuration() const
{
	const AkUInt16 uLoopCount = m_pCtx->m_LoopCount;
	if( !uLoopCount )
	{
		return 0.f;
	}
	return (AkReal32)( ( (AkReal64)( m_uPCMLoopEnd - m_uPCMLoopStart + 1 ) * (AkReal64)( uLoopCount - 1 ) + (AkReal64)m_uTotalSamples ) * 1000.0 / (AkReal64)m_pCtx->m_sMediaFormat.uSampleRate );
}

/** 2012 0x955940 (akvplsrcnode.h inline). */
AKRESULT CAkSrcBaseEx::OnLoopComplete( bool in_bEndOfFile )
{
	if( m_uLoopCnt > 1 )
	{
		--m_uLoopCnt;
	}
	return in_bEndOfFile ? AK_NoMoreData : AK_DataReady;
}

/** 2012 0x95ecd0. */
void CAkSrcBaseEx::TimeSkipMarkersAndPosition( AkUInt32 in_ulCurrSampleOffset, AkUInt32 in_uSkippedSamples, AkUInt32 in_uFileEnd )
{
	m_markers.TimeSkipMarkers( m_pCtx, in_ulCurrSampleOffset, in_uSkippedSamples, in_uFileEnd );
	if( m_pCtx->m_uRegisteredNotif & AK_EnableGetSourcePlayPosition )
	{
		AkBufferPosInformation posInfo;
		posInfo.uSampleRate = m_pCtx->m_sMediaFormat.uSampleRate;
		posInfo.uStartPos = in_ulCurrSampleOffset;
		posInfo.uFileEnd = in_uFileEnd;
		posInfo.fLastRate = 1.f;
		g_pPositionRepository->UpdatePositionInfo( m_pCtx->GetPlayingID(), &posInfo, this );
	}
}

/** 2012 0x95ed60: a virtual voice advances its position without decoding. */
AKRESULT CAkSrcBaseEx::TimeSkip( AkUInt32 & io_uFrames )
{
	const AkUInt32 uCurSample = m_uCurSample;
	const AkUInt16 uLoopCnt = m_uLoopCnt;
	AKRESULT eResult = AK_DataReady;
	m_uCurSample = io_uFrames + uCurSample;
	if( uLoopCnt == 1 )
	{
		if( m_uCurSample >= m_uTotalSamples )
		{
			io_uFrames = m_uTotalSamples - uCurSample;
			eResult = AK_NoMoreData;
		}
	}
	else if( m_uCurSample > m_uPCMLoopEnd )
	{
		if( uLoopCnt )
		{
			m_uLoopCnt = uLoopCnt - 1;
		}
		io_uFrames = m_uPCMLoopEnd - uCurSample + 1;
		m_uCurSample = m_uPCMLoopStart;
	}
	TimeSkipMarkersAndPosition( uCurSample, io_uFrames, m_uTotalSamples );
	return eResult;
}

/** 2012 0x95edd0: hands the decoded frames down the pipeline and advances the position, looping or ending. */
void CAkSrcBaseEx::SubmitBufferAndUpdate( void * in_pData, AkUInt16 in_uNumSamplesProduced, AkUInt32 in_uSampleRate, AkChannelMask in_channelMask, AkVPLState & io_state )
{
	if( !in_uNumSamplesProduced )
	{
		io_state.buffer.uValidFrames = 0;
		io_state.result = AK_NoDataReady;
		return;
	}
	io_state.buffer.pData = in_pData;
	io_state.buffer.uMaxFrames = in_uNumSamplesProduced;
	io_state.buffer.uValidFrames = in_uNumSamplesProduced;
	io_state.buffer.uChannelMask = in_channelMask;
	m_markers.CopyRelevantMarkers( m_pCtx, io_state.buffer, m_uCurSample );
	if( m_pCtx->m_uRegisteredNotif & AK_EnableGetSourcePlayPosition )
	{
		io_state.buffer.posInfo.uSampleRate = in_uSampleRate;
		io_state.buffer.posInfo.uStartPos = m_uCurSample;
		io_state.buffer.posInfo.uFileEnd = m_uTotalSamples;
	}
	m_uCurSample += in_uNumSamplesProduced;
	AKRESULT eResult = AK_DataReady;
	if( m_uLoopCnt == 1 )
	{
		if( m_uCurSample >= m_uTotalSamples )
		{
			eResult = OnLoopComplete( true );
		}
	}
	else if( m_uCurSample > m_uPCMLoopEnd )
	{
		m_uCurSample = m_uPCMLoopStart;
		eResult = OnLoopComplete( false );
	}
	io_state.result = eResult;
}

/** 2012 0x95ee90: the PBI's pending seek as a sample offset into the file (and the loops left), snapped to a marker if asked. */
AkUInt32 CAkSrcBaseEx::GetSourceOffset()
{
	CAkPBI * pCtx = m_pCtx;
	AkUInt32 uAbsoluteSourceOffset;
	bool bSnapToMarker;
	if( pCtx->m_bSeekRelativeToDuration )
	{
		const AkReal32 fSourceDuration = pCtx->m_LoopCount ? GetDuration() : (AkReal32)( (AkReal64)m_uTotalSamples * 1000.0 / (AkReal64)pCtx->m_sMediaFormat.uSampleRate );
		uAbsoluteSourceOffset = pCtx->GetSeekPosition( fSourceDuration, bSnapToMarker );
	}
	else
	{
		bSnapToMarker = pCtx->m_bSnapSeekToMarker;
		const AkUInt32 uSeekFrequency = pCtx->m_bTargetIsFeedback ? 375 : AkAudioLibSettings::g_pipelineCoreFrequency;
		uAbsoluteSourceOffset = (AkUInt32)( (AkUInt64)pCtx->m_sMediaFormat.uSampleRate * (AkUInt64)pCtx->m_uSeekPosition / uSeekFrequency );
	}
	AkUInt32 uRelativeSourceOffset;
	AbsoluteToRelativeSourceOffset( uAbsoluteSourceOffset, m_uPCMLoopStart, m_uPCMLoopEnd, pCtx->m_LoopCount, uRelativeSourceOffset, m_uLoopCnt );
	if( !bSnapToMarker )
	{
		return uRelativeSourceOffset;
	}
	const AkAudioMarker * pMarker = m_markers.GetClosestMarker( uRelativeSourceOffset );
	if( !pMarker )
	{
		return uRelativeSourceOffset;
	}
	AbsoluteToRelativeSourceOffset( pMarker->dwPosition, m_uPCMLoopStart, m_uPCMLoopEnd, m_uLoopCnt, uRelativeSourceOffset, m_uLoopCnt );
	return uRelativeSourceOffset;
}
