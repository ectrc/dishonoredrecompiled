/*=============================================================================
	AkSrcFileADPCM.cpp - CAkSrcFileADPCM (aksrcfileadpcm.cpp, 2012 0x956580..0x956ab0; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). ParseHeader's stream throughput is retail's to the byte: it multiplies
	nBlockAlign by the 32-bit word at &nChannels (nChannels plus the low half of nSamplesPerSec, read from the 2012
	shipping executable's code at 0x9569a8), not by the sample rate; it is only a scheduling heuristic.
=============================================================================*/
#include "AkSrcFileADPCM.h"
#include "AkADPCMCodec.h"
#include "AkFileParser.h"
#include "AkLEngine.h"
#include "AkPBI.h"

/** 2012 0x956580. */
CAkSrcFileADPCM::CAkSrcFileADPCM( CAkPBI * in_pCtx )
	: CAkSrcFileBase( in_pCtx )
	, m_pOutBuffer( NULL )
	, m_wExtraSize( 0 )
{
}

/** 2012 0x9565b0: first the block gathered across the previous stream buffer, then the whole blocks of this one. */
void CAkSrcFileADPCM::GetBuffer( AkVPLState & io_state )
{
	if( m_bWaitForBuffering )
	{
		const AKRESULT eResult = AK::SrcFileServices::IsPrebufferingReady( m_pStream, m_ulSizeLeft );
		if( eResult != AK_DataReady )
		{
			io_state.result = eResult;
			return;
		}
		m_bWaitForBuffering = 0;
	}
	if( !m_ulSizeLeft )
	{
		const AKRESULT eResult = FetchStreamBuffer();
		if( eResult != AK_DataReady )
		{
			io_state.result = eResult;
			return;
		}
	}

	const AkAudioFormat & format = m_pCtx->m_sMediaFormat;
	const AkUInt32 uBlockAlign = format.uBlockAlign;
	const AkUInt32 uChannelMask = format.uChannelMask;
	const AkUInt32 uNumChannels = format.GetNumChannels();
	m_pOutBuffer = (AkUInt8 *)CAkLEngine::GetCachedAudioBuffer( uBlockAlign * LE_MAX_FRAMES_PER_BUFFER );
	if( !m_pOutBuffer )
	{
		io_state.result = AK_Fail;
		return;
	}

	AkUInt8 * pOutBuffer = m_pOutBuffer;
	const AkUInt32 ulPCMBlockAlign = uBlockAlign * ADPCM_SAMPLES_PER_BLOCK;
	AkUInt32 uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
	if( m_wExtraSize )
	{
		memcpy( m_ExtraBlock + m_wExtraSize, m_pNextAddress, m_uInputBlockSize - m_wExtraSize );
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			CAkADPCMCodec::Decode( m_ExtraBlock + i * ADPCM_BLOCK_SIZE, pOutBuffer + i * sizeof( AkInt16 ), 1, m_uInputBlockSize, uNumChannels );
		}
		const AkUInt32 uConsumed = m_uInputBlockSize - m_wExtraSize;
		m_pNextAddress += uConsumed;
		m_ulSizeLeft -= uConsumed;
		pOutBuffer += ulPCMBlockAlign;
		m_wExtraSize = 0;
		uMaxFrames = LE_MAX_FRAMES_PER_BUFFER - ADPCM_SAMPLES_PER_BLOCK;
	}

	AkUInt32 ulADPCMBlocksToProcess = m_ulSizeLeft / m_uInputBlockSize;
	if( ulADPCMBlocksToProcess >= uMaxFrames / ADPCM_SAMPLES_PER_BLOCK )
	{
		ulADPCMBlocksToProcess = uMaxFrames / ADPCM_SAMPLES_PER_BLOCK;
	}
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		CAkADPCMCodec::Decode( m_pNextAddress + i * ADPCM_BLOCK_SIZE, pOutBuffer + i * sizeof( AkInt16 ), ulADPCMBlocksToProcess, m_uInputBlockSize, uNumChannels );
	}
	const AkUInt16 uFramesProduced = (AkUInt16)( ( pOutBuffer + ulPCMBlockAlign * ulADPCMBlocksToProcess - m_pOutBuffer ) / uBlockAlign );
	m_pNextAddress += ulADPCMBlocksToProcess * m_uInputBlockSize;
	m_ulSizeLeft -= ulADPCMBlocksToProcess * m_uInputBlockSize;
	if( m_ulSizeLeft < m_uInputBlockSize )
	{
		m_wExtraSize = (AkUInt16)m_ulSizeLeft;
		memcpy( m_ExtraBlock, m_pNextAddress, m_ulSizeLeft );
		m_pNextAddress += m_ulSizeLeft;
		m_ulSizeLeft = 0;
		ReleaseStreamBuffer();
	}
	SubmitBufferAndUpdate( m_pOutBuffer, uFramesProduced, format.uSampleRate, uChannelMask, io_state );
}

/** 2012 0x956800. */
void CAkSrcFileADPCM::StopStream()
{
	ReleaseBuffer();
	CAkSrcFileBase::StopStream();
}

/** 2012 0x956820. */
void CAkSrcFileADPCM::ReleaseBuffer()
{
	if( m_pOutBuffer )
	{
		CAkLEngine::ReleaseCachedAudioBuffer( m_pCtx->m_sMediaFormat.uBlockAlign * LE_MAX_FRAMES_PER_BUFFER, m_pOutBuffer );
		m_pOutBuffer = NULL;
	}
}

/** 2012 0x956850. */
void CAkSrcFileADPCM::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	CAkSrcFileBase::VirtualOn( eBehavior );
	if( eBehavior <= AkVirtualQueueBehavior_FromElapsedTime )
	{
		m_wExtraSize = 0;
	}
}

/** 2012 0x956880: the media format, the loop region in samples and in whole file blocks, and the stream heuristics. */
AKRESULT CAkSrcFileADPCM::ParseHeader( AkUInt8 * in_pBuffer )
{
	AkFileParser::FormatInfo fmtInfo;
	const AKRESULT eResult = AkFileParser::Parse( in_pBuffer, m_ulSizeLeft, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const WaveFormatExtensible * pFormat = fmtInfo.pFormat;
	if( pFormat->wFormatTag != AK_WAVE_FORMAT_ADPCM )
	{
		return AK_InvalidFile;
	}
	AkAudioFormat format;
	format.SetAll( pFormat->nSamplesPerSec, pFormat->dwChannelMask, 16, pFormat->nChannels * sizeof( AkInt16 ), AK_INT, AK_INTERLEAVED );
	m_pCtx->SetMediaFormat( format );

	m_uInputBlockSize = pFormat->nBlockAlign;
	m_uTotalSamples = m_uDataSize * ADPCM_SAMPLES_PER_BLOCK / pFormat->nBlockAlign;
	const AkUInt32 ulEndOfData = m_uDataOffset + m_uDataSize;
	if( !m_uPCMLoopEnd || m_uLoopCnt == 1 )
	{
		m_ulLoopStart = m_uDataOffset;
		m_ulLoopEnd = ulEndOfData;
		m_uPCMLoopEnd = m_uDataSize / m_uInputBlockSize * ADPCM_SAMPLES_PER_BLOCK - 1;
	}
	else
	{
		m_ulLoopStart = m_uDataOffset + m_uInputBlockSize * ( m_uPCMLoopStart / ADPCM_SAMPLES_PER_BLOCK );
		m_ulLoopEnd = m_uDataOffset + m_uInputBlockSize * ( ( m_uPCMLoopEnd + 1 ) / ADPCM_SAMPLES_PER_BLOCK );
		if( m_uPCMLoopEnd < m_uPCMLoopStart || m_ulLoopStart > ulEndOfData || m_ulLoopEnd > ulEndOfData )
		{
			return AK_InvalidFile;
		}
	}

	AkAutoStmHeuristics heuristics;
	m_pStream->GetHeuristics( heuristics );
	heuristics.fThroughput = (AkReal32)( (AkReal64)pFormat->nBlockAlign * (AkReal64)*(const AkUInt32 *)&pFormat->nChannels * 0.000015625001 );
	if( m_uLoopCnt != 1 )
	{
		heuristics.uLoopStart = m_ulLoopStart;
		heuristics.uLoopEnd = m_ulLoopEnd;
	}
	heuristics.priority = (AkPriority)m_pCtx->GetPriority();
	m_pStream->SetHeuristics( heuristics );
	return m_pStream->SetMinimalBufferSize( ADPCM_BLOCK_SIZE * pFormat->nChannels );
}

/** 2012 0x956a30: bytes per ms (one 36-byte block per channel every 64 samples). */
AkReal32 CAkSrcFileADPCM::GetThroughput( const AkAudioFormat & in_rFormat )
{
	return (AkReal32)( (AkReal64)( ADPCM_BLOCK_SIZE * in_rFormat.GetNumChannels() * in_rFormat.uSampleRate ) * 0.000015625001 );
}

/** 2012 0x956a80. */
AKRESULT CAkSrcFileADPCM::FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset )
{
	out_uSeekedSample = in_uDesiredSample / ADPCM_SAMPLES_PER_BLOCK * ADPCM_SAMPLES_PER_BLOCK;
	out_uFileOffset = m_uDataOffset + ( in_uDesiredSample / ADPCM_SAMPLES_PER_BLOCK ) * m_uInputBlockSize;
	return AK_Success;
}

/** 2012 0x956ab0. */
AKRESULT CAkSrcFileADPCM::ChangeSourcePosition()
{
	m_wExtraSize = 0;
	return CAkSrcFileBase::ChangeSourcePosition();
}
