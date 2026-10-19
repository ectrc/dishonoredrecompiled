/*=============================================================================
	AkSrcBankADPCM.cpp - CAkSrcBankADPCM (aksrcbankadpcm.cpp, 2012 0x956180..0x956510; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). Whole 64-sample blocks are decoded into a CAkLEngine cached buffer of
	1024 interleaved 16-bit frames; a seek lands on a block and leaves the rest in the PBI as the source offset remainder.
=============================================================================*/
#include "AkSrcBankADPCM.h"
#include "AkADPCMCodec.h"
#include "AkFileParser.h"
#include "AkLEngine.h"
#include "AkPBI.h"

/** 2012 0x956180. */
CAkSrcBankADPCM::CAkSrcBankADPCM( CAkPBI * in_pCtx )
	: CAkSrcBaseEx( in_pCtx )
	, m_pucData( NULL )
	, m_pOutBuffer( NULL )
{
}

/** 2012 0x9561b0. */
void CAkSrcBankADPCM::StopStream()
{
	ReleaseBuffer();
	CAkSrcBaseEx::StopStream();
}

/** 2012 0x9561d0. */
void CAkSrcBankADPCM::ReleaseBuffer()
{
	if( m_pOutBuffer )
	{
		CAkLEngine::ReleaseCachedAudioBuffer( m_pCtx->m_sMediaFormat.GetNumChannels() * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkInt16 ), m_pOutBuffer );
		m_pOutBuffer = NULL;
	}
}

/** 2012 0x956210. */
AKRESULT CAkSrcBankADPCM::OnLoopComplete( bool in_bEndOfFile )
{
	m_pucData = m_pCtx->m_pDataPtr + m_uDataOffset + m_uInputBlockSize * ( m_uCurSample / ADPCM_SAMPLES_PER_BLOCK );
	return CAkSrcBaseEx::OnLoopComplete( in_bEndOfFile );
}

/** 2012 0x956250. */
AKRESULT CAkSrcBankADPCM::SeekToSourceOffset()
{
	if( !m_pCtx->m_bSeekDirty )
	{
		return AK_Success;
	}
	const AkUInt32 uSourceOffset = GetSourceOffset();
	m_uCurSample = uSourceOffset / ADPCM_SAMPLES_PER_BLOCK * ADPCM_SAMPLES_PER_BLOCK;
	m_pCtx->SetSourceOffsetRemainder( uSourceOffset - m_uCurSample );
	if( m_uCurSample >= m_uTotalSamples )
	{
		return AK_Fail;
	}
	m_pucData = m_pCtx->m_pDataPtr + m_uDataOffset + m_uInputBlockSize * ( m_uCurSample / ADPCM_SAMPLES_PER_BLOCK );
	return AK_Success;
}

/** 2012 0x9562b0. */
AKRESULT CAkSrcBankADPCM::ChangeSourcePosition()
{
	return SeekToSourceOffset();
}

/** 2012 0x9562f0. */
AKRESULT CAkSrcBankADPCM::StartStream()
{
	AkUInt8 * pvBuffer = m_pCtx->m_pDataPtr;
	if( !pvBuffer )
	{
		return AK_Fail;
	}
	AkFileParser::FormatInfo fmtInfo;
	if( AkFileParser::Parse( pvBuffer, m_pCtx->m_uDataSize, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL ) != AK_Success
		|| fmtInfo.pFormat->wFormatTag != AK_WAVE_FORMAT_ADPCM )
	{
		return AK_InvalidFile;
	}
	const WaveFormatExtensible * pFormat = fmtInfo.pFormat;
	AkAudioFormat format;
	format.SetAll( pFormat->nSamplesPerSec, pFormat->dwChannelMask, 16, pFormat->nChannels * sizeof( AkInt16 ), AK_INT, AK_INTERLEAVED );
	m_pCtx->SetMediaFormat( format );

	m_uInputBlockSize = pFormat->nBlockAlign;
	m_uTotalSamples = m_uDataSize * ADPCM_SAMPLES_PER_BLOCK / m_uInputBlockSize;
	if( !m_uPCMLoopEnd || m_uLoopCnt == 1 )
	{
		m_uPCMLoopEnd = m_uTotalSamples - 1;
	}
	m_uCurSample = 0;
	m_pucData = pvBuffer + m_uDataOffset;
	if( m_pCtx->m_bSeekDirty )
	{
		return SeekToSourceOffset();
	}
	return AK_Success;
}

/** 2012 0x9563f0: whole blocks only, up to the end of the file or of the loop. */
void CAkSrcBankADPCM::GetBuffer( AkVPLState & io_state )
{
	const AkAudioFormat & format = m_pCtx->m_sMediaFormat;
	const AkUInt32 uChannelMask = format.uChannelMask;
	const AkUInt32 uNumChannels = format.GetNumChannels();
	m_pOutBuffer = (AkUInt8 *)CAkLEngine::GetCachedAudioBuffer( uNumChannels * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkInt16 ) );
	if( !m_pOutBuffer )
	{
		io_state.result = AK_Fail;
		return;
	}
	AkUInt16 uMaxFrames = io_state.buffer.uMaxFrames;
	const AkUInt32 uEndSample = ( m_uLoopCnt == 1 ) ? m_uTotalSamples : m_uPCMLoopEnd + 1;
	if( m_uCurSample + uMaxFrames > uEndSample )
	{
		uMaxFrames = (AkUInt16)( uEndSample - m_uCurSample );
	}
	const AkUInt32 uNumBlocks = uMaxFrames / ADPCM_SAMPLES_PER_BLOCK;
	const AkUInt16 uNumFrames = (AkUInt16)( uNumBlocks * ADPCM_SAMPLES_PER_BLOCK );
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		CAkADPCMCodec::Decode( m_pucData + i * ADPCM_BLOCK_SIZE, m_pOutBuffer + i * sizeof( AkInt16 ), uNumBlocks, m_uInputBlockSize, uNumChannels );
	}
	m_pucData += uNumBlocks * m_uInputBlockSize;
	SubmitBufferAndUpdate( m_pOutBuffer, uNumFrames, format.uSampleRate, uChannelMask, io_state );
}

/** 2012 0x956510. */
AKRESULT CAkSrcBankADPCM::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	if( eBehavior == AkVirtualQueueBehavior_FromBeginning )
	{
		m_uCurSample = 0;
		m_pucData = m_pCtx->m_pDataPtr + m_uDataOffset;
		m_uLoopCnt = m_pCtx->m_LoopCount;
		return AK_Success;
	}
	if( eBehavior == AkVirtualQueueBehavior_FromElapsedTime )
	{
		if( in_bUseSourceOffset )
		{
			return SeekToSourceOffset();
		}
		m_pucData = m_pCtx->m_pDataPtr + m_uDataOffset + m_uInputBlockSize * ( m_uCurSample / ADPCM_SAMPLES_PER_BLOCK );
	}
	return AK_Success;
}
