/*=============================================================================
	AkSrcBankPCM.cpp - CAkSrcBankPCM (aksrcbankpcm.cpp, 2012 0x955920..0x955b70; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). The media is the PBI's m_pDataPtr / m_uDataSize; the buffer handed
	down is interleaved integer PCM pointing into the media (no copy).
=============================================================================*/
#include "AkSrcBankPCM.h"
#include "AkFileParser.h"
#include "AkPBI.h"

/** 2012 0x955920. */
CAkSrcBankPCM::CAkSrcBankPCM( CAkPBI * in_pCtx )
	: CAkSrcBaseEx( in_pCtx )
	, m_pDataStart( NULL )
{
}

/** 2012 0x955970. */
AKRESULT CAkSrcBankPCM::ChangeSourcePosition()
{
	m_uCurSample = GetSourceOffset();
	m_pCtx->SetSourceOffsetRemainder( 0 );
	return m_uCurSample < m_uTotalSamples ? AK_Success : AK_Fail;
}

/** 2012 0x9559d0. */
AKRESULT CAkSrcBankPCM::StartStream()
{
	AkUInt8 * pDataPtr = m_pCtx->m_pDataPtr;
	const AkUInt32 ulBufferSize = m_pCtx->m_uDataSize;
	if( !pDataPtr )
	{
		return AK_Fail;
	}
	AkFileParser::FormatInfo fmtInfo;
	if( AkFileParser::Parse( pDataPtr, ulBufferSize, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL ) != AK_Success
		|| fmtInfo.pFormat->wFormatTag != AK_WAVE_FORMAT_EXTENSIBLE )
	{
		return AK_InvalidFile;
	}
	const WaveFormatExtensible * pFormat = fmtInfo.pFormat;
	AkAudioFormat format;
	format.SetAll( pFormat->nSamplesPerSec, pFormat->dwChannelMask, pFormat->wBitsPerSample, pFormat->nBlockAlign, AK_INT, AK_INTERLEAVED );
	m_pCtx->SetMediaFormat( format );

	m_pDataStart = pDataPtr + m_uDataOffset;
	m_uTotalSamples = m_uDataSize / pFormat->nBlockAlign;
	if( !m_uPCMLoopEnd || m_uLoopCnt == 1 )
	{
		m_uPCMLoopEnd = m_uTotalSamples - 1;
	}
	if( m_uPCMLoopEnd < m_uPCMLoopStart || m_uPCMLoopEnd >= m_uTotalSamples || ulBufferSize != m_uDataOffset + m_uDataSize )
	{
		return AK_Fail;
	}
	m_uCurSample = 0;
	if( m_pCtx->m_bSeekDirty )
	{
		return ChangeSourcePosition();
	}
	return AK_Success;
}

/** 2012 0x955b10: up to the end of the file, or of the loop while loops remain. */
void CAkSrcBankPCM::GetBuffer( AkVPLState & io_state )
{
	AkUInt16 uMaxFrames = io_state.buffer.uMaxFrames;
	const AkUInt32 uEndSample = ( m_uLoopCnt == 1 ) ? m_uTotalSamples : m_uPCMLoopEnd + 1;
	if( m_uCurSample + uMaxFrames > uEndSample )
	{
		uMaxFrames = (AkUInt16)( uEndSample - m_uCurSample );
	}
	const AkAudioFormat & format = m_pCtx->m_sMediaFormat;
	SubmitBufferAndUpdate( m_pDataStart + m_uCurSample * format.uBlockAlign, uMaxFrames, format.uSampleRate, format.uChannelMask, io_state );
}

/** 2012 0x955b70. */
AKRESULT CAkSrcBankPCM::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	if( eBehavior == AkVirtualQueueBehavior_FromBeginning )
	{
		m_uCurSample = 0;
		m_uLoopCnt = m_pCtx->m_LoopCount;
		return AK_Success;
	}
	if( in_bUseSourceOffset && eBehavior == AkVirtualQueueBehavior_FromElapsedTime )
	{
		return ChangeSourcePosition();
	}
	return AK_Success;
}
