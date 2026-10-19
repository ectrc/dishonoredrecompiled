/*=============================================================================
	AkSrcFilePCM.cpp - CAkSrcFilePCM (aksrcfilepcm.cpp, 2012 0x955c20..0x956110; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). The buffer handed down points into the stream buffer (or the stitch
	buffer for a frame split across two stream buffers); ReleaseBuffer consumes it.
=============================================================================*/
#include "AkSrcFilePCM.h"
#include "AkFileParser.h"
#include "AkPBI.h"

/** 2012 0x955c20. */
CAkSrcFilePCM::CAkSrcFilePCM( CAkPBI * in_pCtx )
	: CAkSrcFileBase( in_pCtx )
	, m_pStitchBuffer( NULL )
	, m_uNumBytesBuffered( 0 )
	, m_uSizeToRelease( 0 )
{
}

/** 2012 0x955c80. */
void CAkSrcFilePCM::StopStream()
{
	if( m_pStitchBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pStitchBuffer );
		m_pStitchBuffer = NULL;
		m_uNumBytesBuffered = 0;
	}
	CAkSrcFileBase::StopStream();
}

/** 2012 0x955cb0. */
void CAkSrcFilePCM::GetBuffer( AkVPLState & io_state )
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

	const AkUInt32 uBlockAlign = GetBlockAlign();
	const AkUInt32 uFramesLeft = m_ulSizeLeft / uBlockAlign;
	if( !m_ulSizeLeft && m_bIsLastStmBuffer )
	{
		io_state.result = AK_Fail;
		return;
	}
	const AkAudioFormat & format = m_pCtx->m_sMediaFormat;
	const AkUInt16 uMaxFrames = (AkUInt16)( io_state.buffer.uMaxFrames >= uFramesLeft ? uFramesLeft : io_state.buffer.uMaxFrames );

	if( m_uNumBytesBuffered )
	{
		const AkUInt32 uMissingBytes = uBlockAlign - m_uNumBytesBuffered;
		if( uMissingBytes > m_ulSizeLeft )
		{
			io_state.result = AK_Fail;
			return;
		}
		memcpy( m_pStitchBuffer + m_uNumBytesBuffered, m_pNextAddress, uMissingBytes );
		m_uNumBytesBuffered = 0;
		m_uSizeToRelease = (AkUInt16)uMissingBytes;
		SubmitBufferAndUpdate( m_pStitchBuffer, 1, format.uSampleRate, format.uChannelMask, io_state );
		return;
	}

	if( m_pStitchBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pStitchBuffer );
		m_pStitchBuffer = NULL;
	}
	AkUInt8 * pData = m_pNextAddress;
	m_uSizeToRelease = uBlockAlign * uMaxFrames;
	const AkUInt32 uBytesLeftAfter = m_ulSizeLeft - m_uSizeToRelease;
	if( uBytesLeftAfter < uBlockAlign && uBytesLeftAfter )
	{
		m_pStitchBuffer = (AkUInt8 *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uBlockAlign );
		if( !m_pStitchBuffer )
		{
			io_state.result = AK_Fail;
			return;
		}
		m_uNumBytesBuffered = (AkUInt16)uBytesLeftAfter;
		memcpy( m_pStitchBuffer, m_pNextAddress + m_uSizeToRelease, (AkUInt16)uBytesLeftAfter );
		m_uSizeToRelease += m_uNumBytesBuffered;
	}
	SubmitBufferAndUpdate( pData, uMaxFrames, format.uSampleRate, format.uChannelMask, io_state );
}

/** 2012 0x955e80. */
void CAkSrcFilePCM::ReleaseBuffer()
{
	m_ulSizeLeft -= m_uSizeToRelease;
	m_pNextAddress += m_uSizeToRelease;
	m_uSizeToRelease = 0;
	if( !m_ulSizeLeft )
	{
		ReleaseStreamBuffer();
	}
}

/** 2012 0x955eb0. */
void CAkSrcFilePCM::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	if( m_pStitchBuffer && eBehavior <= AkVirtualQueueBehavior_FromElapsedTime )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pStitchBuffer );
		m_pStitchBuffer = NULL;
		m_uNumBytesBuffered = 0;
	}
	CAkSrcFileBase::VirtualOn( eBehavior );
}

/** 2012 0x955f00: the media format, the loop region in samples and in file bytes, and the stream heuristics. */
AKRESULT CAkSrcFilePCM::ParseHeader( AkUInt8 * in_pBuffer )
{
	AkFileParser::FormatInfo fmtInfo;
	const AKRESULT eResult = AkFileParser::Parse( in_pBuffer, m_ulSizeLeft, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const WaveFormatExtensible * pFormat = fmtInfo.pFormat;
	if( pFormat->wFormatTag != AK_WAVE_FORMAT_EXTENSIBLE )
	{
		return AK_InvalidFile;
	}
	AkAudioFormat format;
	format.SetAll( pFormat->nSamplesPerSec, pFormat->dwChannelMask, pFormat->wBitsPerSample, pFormat->nBlockAlign, AK_INT, AK_INTERLEAVED );
	m_pCtx->SetMediaFormat( format );

	m_uTotalSamples = m_uDataSize / pFormat->nBlockAlign;
	const AkUInt32 ulEndOfData = m_uDataOffset + m_uDataSize;
	if( !m_uPCMLoopEnd || m_uLoopCnt == 1 )
	{
		m_ulLoopEnd = ulEndOfData;
		m_ulLoopStart = m_uDataOffset;
		m_uPCMLoopEnd = m_uDataSize / pFormat->nBlockAlign - 1;
	}
	else
	{
		m_ulLoopStart = m_uDataOffset + pFormat->nBlockAlign * m_uPCMLoopStart;
		m_ulLoopEnd = m_uDataOffset + pFormat->nBlockAlign * ( m_uPCMLoopEnd + 1 );
		if( m_uPCMLoopEnd < m_uPCMLoopStart || m_ulLoopStart > ulEndOfData || m_ulLoopEnd > ulEndOfData )
		{
			return AK_InvalidFile;
		}
	}

	AkAutoStmHeuristics heuristics;
	m_pStream->GetHeuristics( heuristics );
	heuristics.fThroughput = (AkReal32)( (AkReal64)( pFormat->nSamplesPerSec * pFormat->nBlockAlign ) * 0.001 );
	if( m_uLoopCnt != 1 )
	{
		heuristics.uLoopStart = m_ulLoopStart;
		heuristics.uLoopEnd = m_ulLoopEnd;
	}
	heuristics.priority = (AkPriority)m_pCtx->GetPriority();
	m_pStream->SetHeuristics( heuristics );
	return m_pStream->SetMinimalBufferSize( pFormat->nBlockAlign );
}

/** 2012 0x9560a0: bytes per ms. */
AkReal32 CAkSrcFilePCM::GetThroughput( const AkAudioFormat & in_rFormat )
{
	return (AkReal32)( (AkReal64)( in_rFormat.uSampleRate * GetBlockAlign() ) * 0.001 );
}

/** 2012 0x9560d0. */
AKRESULT CAkSrcFilePCM::FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset )
{
	out_uSeekedSample = in_uDesiredSample;
	out_uFileOffset = m_uDataOffset + in_uDesiredSample * GetBlockAlign();
	return AK_Success;
}

/** 2012 0x956100. */
AkUInt16 CAkSrcFilePCM::GetBlockAlign() const
{
	return (AkUInt16)m_pCtx->m_sMediaFormat.uBlockAlign;
}

/** 2012 0x956110. */
AKRESULT CAkSrcFilePCM::ChangeSourcePosition()
{
	m_uSizeToRelease = 0;
	if( m_pStitchBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pStitchBuffer );
		m_pStitchBuffer = NULL;
		m_uNumBytesBuffered = 0;
	}
	return CAkSrcFileBase::ChangeSourcePosition();
}
