/*=============================================================================
	AkSrcFileVorbis.cpp - CAkSrcFileVorbis and CreateVorbisFilePlugin (aksrcfilevorbis.cpp, 2012 0x9750c0..0x975e70;
	2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). The streamed file is the same as the in-memory one (seek table, setup
	packet, size-prefixed audio packets); the header packets may straddle stream buffers (GetNextPacket gathers them),
	and an audio packet that straddles two buffers is decoded from the stitch buffer.
=============================================================================*/
#include "AkSrcFileVorbis.h"
#include "AkVorbisCodebookMgr.h"
#include "AkFileParser.h"
#include "AkLEngine.h"
#include "AkPBI.h"
#include <AK/Plugin/AkVorbisFactory.h>

/** 2012 0x975e00. */
IAkSoftwareCodec * CreateVorbisFilePlugin( void * in_pCtx )
{
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcFileVorbis ) );
	return pMem ? AkPlacementNew( pMem ) CAkSrcFileVorbis( (CAkPBI *)in_pCtx ) : NULL;
}

/** 2012 0x975790. */
CAkSrcFileVorbis::CAkSrcFileVorbis( CAkPBI * in_pCtx )
	: CAkSrcFileBase( in_pCtx )
	, m_pStitchStreamBuffer( NULL )
	, m_uStitchBufferEndOffset( 0 )
	, m_uStitchBufferValidDataSize( 0 )
	, m_uStitchBufferLeft( 0 )
	, m_pOggPacketData( NULL )
	, m_uPacketDataGathered( 0 )
	, m_uPacketHeaderGathered( 0 )
{
	m_OggPacketHeader.uPacketSize = 0;
	memset( &m_VorbisState, 0, sizeof( m_VorbisState ) );
}

/** 2012 0x975e30 (the deleting destructor; no rva of its own, read from the shipping executable's bytes). */
CAkSrcFileVorbis::~CAkSrcFileVorbis()
{
	ReleaseBuffer();
	if( m_VorbisState.TremorInfo.VorbisDSPState.csi )
	{
		g_VorbisCodebookMgr.ReleaseCodebook( m_VorbisState );
	}
}

/** 2012 0x9750c0. */
void CAkSrcFileVorbis::VorbisDSPRestart( AkUInt16 in_uSrcOffsetRemainder )
{
	const AkUInt16 uExtraSamplesEnd = ( m_uLoopCnt == 1 ) ? m_VorbisState.VorbisInfo.uLastGranuleExtra : m_VorbisState.VorbisInfo.LoopInfo.uLoopEndExtra;
	vorbis_dsp_restart( &m_VorbisState.TremorInfo.VorbisDSPState, in_uSrcOffsetRemainder, uExtraSamplesEnd );
	m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = PACKET_STREAM;
}

/** 2012 0x975100: no frames out of a packet that did consume input is not starvation. */
void CAkSrcFileVorbis::SubmitBufferAndUpdateVorbis( AkVPLState & io_state )
{
	io_state.result = m_VorbisState.TremorInfo.ReturnInfo.eDecoderStatus;
	if( io_state.result == AK_Fail )
	{
		return;
	}
	SubmitBufferAndUpdate( m_VorbisState.TremorInfo.pucData, (AkUInt16)m_VorbisState.TremorInfo.ReturnInfo.uFramesProduced, m_VorbisState.uSampleRate, m_VorbisState.TremorInfo.uChannelMask, io_state );
	if( io_state.result == AK_NoDataReady && m_pNextAddress )
	{
		io_state.result = m_VorbisState.TremorInfo.ReturnInfo.uInputBytesConsumed ? AK_DataReady : AK_Fail;
	}
}

/** 2012 0x975160. */
AKRESULT CAkSrcFileVorbis::OnLoopComplete( bool in_bEndOfFile )
{
	if( !in_bEndOfFile )
	{
		--m_uStreamLoopCntAhead;
	}
	if( m_uLoopCnt > 1 )
	{
		--m_uLoopCnt;
	}
	if( !in_bEndOfFile )
	{
		VorbisDSPRestart( m_VorbisState.VorbisInfo.LoopInfo.uLoopBeginExtra );
		m_VorbisState.TremorInfo.ReturnInfo.eDecoderStatus = AK_DataReady;
	}
	return in_bEndOfFile ? AK_NoMoreData : AK_DataReady;
}

/** 2012 0x9751e0. */
void CAkSrcFileVorbis::ReleaseBuffer()
{
	if( m_VorbisState.TremorInfo.pucData )
	{
		CAkLEngine::ReleaseCachedAudioBuffer( AkGetNumChannels( m_VorbisState.TremorInfo.uChannelMask ) * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkInt16 ), m_VorbisState.TremorInfo.pucData );
		m_VorbisState.TremorInfo.ReturnInfo.uFramesProduced = 0;
		m_VorbisState.TremorInfo.pucData = NULL;
	}
}

/** 2012 0x975230: if the stream has not looped ahead, the current loop ends on the file's last granule. */
AKRESULT CAkSrcFileVorbis::StopLooping()
{
	if( !m_uStreamLoopCntAhead )
	{
		m_VorbisState.TremorInfo.VorbisDSPState.state.extra_samples_end = m_VorbisState.VorbisInfo.uLastGranuleExtra;
	}
	return CAkSrcFileBase::StopLooping();
}

/** 2012 0x975250. */
AKRESULT CAkSrcFileVorbis::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	const AKRESULT eResult = CAkSrcFileBase::VirtualOff( eBehavior, in_bUseSourceOffset );
	if( eBehavior <= AkVirtualQueueBehavior_FromElapsedTime )
	{
		const AkUInt32 uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
		m_pCtx->SetSourceOffsetRemainder( 0 );
		m_uCurSample += uSrcOffsetRemainder;
		VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
	}
	return eResult;
}

/** 2012 0x9752d0. */
AKRESULT CAkSrcFileVorbis::InitVorbisInfo()
{
	if( m_VorbisState.VorbisInfo.dwSeekTableSize )
	{
		m_VorbisState.pSeekTable = (AkVorbisSeekTableItem *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, m_VorbisState.VorbisInfo.dwSeekTableSize );
		if( !m_VorbisState.pSeekTable )
		{
			return AK_InsufficientMemory;
		}
	}
	m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = INITIALIZED;
	return AK_Success;
}

/** 2012 0x975310. */
AKRESULT CAkSrcFileVorbis::ParseHeader( AkUInt8 * in_pBuffer )
{
	AkFileParser::FormatInfo fmtInfo;
	AKRESULT eResult = AkFileParser::Parse( in_pBuffer, m_ulSizeLeft, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( fmtInfo.pFormat->wFormatTag != AK_WAVE_FORMAT_VORBIS )
	{
		return AK_InvalidFile;
	}
	const WaveFormatVorbis * pFmt = (const WaveFormatVorbis *)fmtInfo.pFormat;
	AkAudioFormat format;
	format.SetAll( pFmt->nSamplesPerSec, pFmt->dwChannelMask, 16, pFmt->nChannels * sizeof( AkInt16 ), AK_INT, AK_INTERLEAVED );
	m_pCtx->SetMediaFormat( format );

	m_uTotalSamples = pFmt->vorbisHeader.dwTotalPCMFrames;
	m_VorbisState.VorbisInfo = pFmt->vorbisHeader;
	m_VorbisState.TremorInfo.uChannelMask = pFmt->dwChannelMask;
	m_VorbisState.uSampleRate = pFmt->nSamplesPerSec;
	if( !m_uPCMLoopStart && !m_uPCMLoopEnd )
	{
		m_uPCMLoopEnd = m_uTotalSamples - 1;
	}
	if( m_uLoopCnt == 1 )
	{
		m_ulLoopStart = m_uDataOffset + m_VorbisState.VorbisInfo.dwVorbisDataOffset;
		m_ulLoopEnd = m_uDataOffset + m_uDataSize;
	}
	else
	{
		m_ulLoopStart = m_uDataOffset + m_VorbisState.VorbisInfo.dwSeekTableSize + m_VorbisState.VorbisInfo.LoopInfo.dwLoopStartPacketOffset;
		m_ulLoopEnd = m_uDataOffset + m_VorbisState.VorbisInfo.dwSeekTableSize + m_VorbisState.VorbisInfo.LoopInfo.dwLoopEndPacketOffset;
	}

	AkAutoStmHeuristics heuristics;
	m_pStream->GetHeuristics( heuristics );
	GetStreamLoopHeuristic( m_uLoopCnt != 1, heuristics );
	heuristics.fThroughput = (AkReal32)( (AkReal64)pFmt->nAvgBytesPerSec * 0.001 );
	heuristics.priority = (AkPriority)m_pCtx->GetPriority();
	m_pStream->SetHeuristics( heuristics );

	eResult = InitVorbisInfo();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return m_pStream->SetMinimalBufferSize( 2 * m_VorbisState.VorbisInfo.uMaxPacketSize );
}

/** 2012 0x9754c0. */
AKRESULT CAkSrcFileVorbis::FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset )
{
	const AKRESULT eResult = VorbisSeek( m_VorbisState, in_uDesiredSample, out_uSeekedSample, out_uFileOffset );
	if( eResult == AK_Fail )
	{
		return eResult;
	}
	out_uFileOffset += m_uDataOffset;
	return AK_Success;
}

/** 2012 0x975500. */
void CAkSrcFileVorbis::FreeStitchBuffer()
{
	if( m_pStitchStreamBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pStitchStreamBuffer );
		m_pStitchStreamBuffer = NULL;
		m_uStitchBufferEndOffset = 0;
		m_uStitchBufferLeft = 0;
		m_uStitchBufferValidDataSize = 0;
	}
}

/** 2012 0x975550: gathers one size-prefixed packet, across as many stream buffers as it takes. */
AKRESULT CAkSrcFileVorbis::GetNextPacket( ogg_packet & out_OggPacket )
{
	for( ;; )
	{
		if( !m_ulSizeLeft )
		{
			if( m_bIsLastStmBuffer )
			{
				return AK_NoMoreData;
			}
			ReleaseStreamBuffer();
			const AKRESULT eResult = FetchStreamBuffer();
			if( eResult != AK_DataReady )
			{
				return eResult;
			}
		}
		if( m_uPacketHeaderGathered < sizeof( OggPacketHeader ) )
		{
			if( !m_ulSizeLeft )
			{
				continue;
			}
			AkUInt32 uCopySize = sizeof( OggPacketHeader ) - m_uPacketHeaderGathered;
			if( m_ulSizeLeft < uCopySize )
			{
				uCopySize = m_ulSizeLeft;
			}
			memcpy( (AkUInt8 *)&m_OggPacketHeader + m_uPacketHeaderGathered, m_pNextAddress, uCopySize );
			m_uPacketHeaderGathered += uCopySize;
			m_pNextAddress += uCopySize;
			m_ulSizeLeft -= uCopySize;
			if( m_uPacketHeaderGathered != sizeof( OggPacketHeader ) )
			{
				continue;
			}
		}
		if( !m_uPacketDataGathered )
		{
			if( m_pOggPacketData )
			{
				AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pOggPacketData );
				m_pOggPacketData = NULL;
			}
			m_pOggPacketData = (AkUInt8 *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, m_OggPacketHeader.uPacketSize );
			if( !m_pOggPacketData )
			{
				return AK_InsufficientMemory;
			}
		}
		if( m_uPacketDataGathered < m_OggPacketHeader.uPacketSize && m_ulSizeLeft )
		{
			AkUInt32 uCopySize = m_OggPacketHeader.uPacketSize - m_uPacketDataGathered;
			if( m_ulSizeLeft < uCopySize )
			{
				uCopySize = m_ulSizeLeft;
			}
			memcpy( m_pOggPacketData + m_uPacketDataGathered, m_pNextAddress, uCopySize );
			m_uPacketDataGathered += uCopySize;
			m_pNextAddress += uCopySize;
			m_ulSizeLeft -= uCopySize;
		}
		if( m_uPacketHeaderGathered == sizeof( OggPacketHeader ) && m_uPacketDataGathered == m_OggPacketHeader.uPacketSize )
		{
			out_OggPacket.buffer.data = m_pOggPacketData;
			out_OggPacket.buffer.size = m_OggPacketHeader.uPacketSize;
			out_OggPacket.e_o_s = 0;
			m_uPacketHeaderGathered = 0;
			m_uPacketDataGathered = 0;
			return AK_DataReady;
		}
	}
}

/** 2012 0x9756e0. */
AKRESULT CAkSrcFileVorbis::ChangeSourcePosition()
{
	const AKRESULT eResult = CAkSrcFileBase::ChangeSourcePosition();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	FreeStitchBuffer();
	const AkUInt32 uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
	m_pCtx->SetSourceOffsetRemainder( 0 );
	m_uCurSample += uSrcOffsetRemainder;
	VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
	return AK_Success;
}

/** 2012 0x975800: once fewer bytes than a packet are left, the rest of the stream buffer is moved to the stitch buffer
	and the next buffer's first packet is appended to it. */
void CAkSrcFileVorbis::GetBuffer( AkVPLState & io_state )
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

	const AkUInt32 uMaxPacketSize = m_VorbisState.VorbisInfo.uMaxPacketSize;
	m_VorbisState.TremorInfo.uRequestedFrames = io_state.buffer.uMaxFrames;
	if( m_ulSizeLeft < uMaxPacketSize + sizeof( OggPacketHeader ) && !m_bIsLastStmBuffer )
	{
		if( m_ulSizeLeft && !m_pStitchStreamBuffer && !m_uStreamLoopCntAhead )
		{
			m_pStitchStreamBuffer = (AkUInt8 *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uMaxPacketSize + m_ulSizeLeft + sizeof( OggPacketHeader ) );
			if( !m_pStitchStreamBuffer )
			{
				io_state.result = AK_Fail;
				return;
			}
			m_uStitchBufferEndOffset = (AkUInt16)m_ulSizeLeft;
			m_uStitchBufferLeft = (AkUInt16)m_ulSizeLeft;
			m_uStitchBufferValidDataSize = (AkUInt16)m_ulSizeLeft;
			memcpy( m_pStitchStreamBuffer, m_pNextAddress, m_ulSizeLeft );
			m_ulSizeLeft = 0;
			m_pNextAddress = NULL;
		}
		if( !m_ulSizeLeft )
		{
			ReleaseStreamBuffer();
			io_state.result = FetchStreamBuffer();
			if( io_state.result == AK_Fail )
			{
				return;
			}
			if( io_state.result == AK_DataReady && m_pStitchStreamBuffer )
			{
				AkUInt32 uCopySize = m_ulSizeLeft;
				if( uMaxPacketSize + sizeof( OggPacketHeader ) < uCopySize )
				{
					uCopySize = uMaxPacketSize + sizeof( OggPacketHeader );
				}
				memcpy( m_pStitchStreamBuffer + m_uStitchBufferEndOffset, m_pNextAddress, uCopySize );
				m_uStitchBufferLeft = (AkUInt16)( m_uStitchBufferLeft + uCopySize );
				m_uStitchBufferValidDataSize = (AkUInt16)( m_uStitchBufferValidDataSize + uCopySize );
			}
		}
	}

	if( !m_VorbisState.TremorInfo.pucData )
	{
		m_VorbisState.TremorInfo.pucData = (AkUInt8 *)CAkLEngine::GetCachedAudioBuffer( m_VorbisState.TremorInfo.VorbisDSPState.channels * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkInt16 ) );
		m_VorbisState.TremorInfo.ReturnInfo.uFramesProduced = 0;
		if( !m_VorbisState.TremorInfo.pucData )
		{
			io_state.result = AK_Fail;
			return;
		}
	}

	const bool bNoMoreInputPackets = m_uStreamLoopCntAhead || m_bIsLastStmBuffer;
	m_VorbisState.TremorInfo.bNoMoreInputPackets = bNoMoreInputPackets;
	if( m_pStitchStreamBuffer )
	{
		m_VorbisState.TremorInfo.bNoMoreInputPackets = bNoMoreInputPackets && !( m_ulSizeLeft + m_uStitchBufferEndOffset - m_uStitchBufferValidDataSize );
		m_VorbisState.TremorInfo.uInputDataSize = m_uStitchBufferLeft;
		DecodeVorbis( &m_VorbisState.TremorInfo, m_VorbisState.VorbisInfo.uMaxPacketSize, m_pStitchStreamBuffer + m_uStitchBufferValidDataSize - m_uStitchBufferLeft, (AkInt16 *)m_VorbisState.TremorInfo.pucData );
		m_uStitchBufferLeft = (AkUInt16)( m_uStitchBufferLeft - (AkUInt16)m_VorbisState.TremorInfo.ReturnInfo.uInputBytesConsumed );
		const AkUInt16 uStitchBufferConsumed = (AkUInt16)( m_uStitchBufferValidDataSize - m_uStitchBufferLeft );
		if( uStitchBufferConsumed >= m_uStitchBufferEndOffset )
		{
			const AkUInt32 uNewBufferConsumed = uStitchBufferConsumed - m_uStitchBufferEndOffset;
			m_pNextAddress += uNewBufferConsumed;
			m_ulSizeLeft -= uNewBufferConsumed;
			FreeStitchBuffer();
		}
	}
	else
	{
		m_VorbisState.TremorInfo.uInputDataSize = m_ulSizeLeft;
		DecodeVorbis( &m_VorbisState.TremorInfo, m_VorbisState.VorbisInfo.uMaxPacketSize, m_pNextAddress, (AkInt16 *)m_VorbisState.TremorInfo.pucData );
		m_pNextAddress += m_VorbisState.TremorInfo.ReturnInfo.uInputBytesConsumed;
		m_ulSizeLeft -= m_VorbisState.TremorInfo.ReturnInfo.uInputBytesConsumed;
	}
	SubmitBufferAndUpdateVorbis( io_state );
}

/** 2012 0x975aa0. */
void CAkSrcFileVorbis::StopStream()
{
	vorbis_dsp_clear( &m_VorbisState.TremorInfo.VorbisDSPState );
	ReleaseBuffer();
	if( m_VorbisState.pSeekTable )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_VorbisState.pSeekTable );
		m_VorbisState.pSeekTable = NULL;
	}
	FreeStitchBuffer();
	if( m_pOggPacketData )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pOggPacketData );
		m_pOggPacketData = NULL;
	}
	CAkSrcFileBase::StopStream();
}

/** 2012 0x975b50. */
void CAkSrcFileVorbis::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	CAkSrcFileBase::VirtualOn( eBehavior );
	if( eBehavior <= AkVirtualQueueBehavior_FromElapsedTime )
	{
		FreeStitchBuffer();
	}
}

/** 2012 0x975bb0: the seek table, then the setup packet, each possibly spread over several stream buffers. */
AKRESULT CAkSrcFileVorbis::DecodeVorbisHeader()
{
	while( m_VorbisState.TremorInfo.ReturnInfo.eDecoderState < PACKET_STREAM )
	{
		if( !m_ulSizeLeft )
		{
			return AK_FormatNotReady;
		}
		if( m_VorbisState.TremorInfo.ReturnInfo.eDecoderState < SEEKTABLEINTIALIZED )
		{
			const AkUInt32 dwSeekTableSize = m_VorbisState.VorbisInfo.dwSeekTableSize;
			if( m_VorbisState.uSeekTableSizeRead < dwSeekTableSize )
			{
				AkUInt32 uCopySize = dwSeekTableSize - m_VorbisState.uSeekTableSizeRead;
				if( m_ulSizeLeft < uCopySize )
				{
					uCopySize = m_ulSizeLeft;
				}
				memcpy( (AkUInt8 *)m_VorbisState.pSeekTable + m_VorbisState.uSeekTableSizeRead, m_pNextAddress, uCopySize );
				m_VorbisState.uSeekTableSizeRead += uCopySize;
				m_pNextAddress += uCopySize;
				m_ulSizeLeft -= uCopySize;
			}
			if( m_VorbisState.uSeekTableSizeRead != dwSeekTableSize )
			{
				continue;
			}
			m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = SEEKTABLEINTIALIZED;
		}
		if( m_VorbisState.TremorInfo.ReturnInfo.eDecoderState == SEEKTABLEINTIALIZED )
		{
			ogg_packet OggPacket;
			switch( GetNextPacket( OggPacket ) )
			{
			case AK_NoDataReady:
				return AK_FormatNotReady;
			case AK_Fail:
			case AK_NoMoreData:
			case AK_InsufficientMemory:
				return AK_Fail;
			default:
				break;
			}
			CAkVorbisAllocator * pAllocator = g_VorbisCodebookMgr.Decodebook( m_VorbisState, m_pCtx, &OggPacket );
			if( !pAllocator )
			{
				return AK_Fail;
			}
			m_VorbisState.TremorInfo.VorbisDSPState.csi = (codec_setup_info *)pAllocator->pStartAddress;
			m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = PACKET_STREAM;
		}
	}
	if( vorbis_dsp_init( &m_VorbisState.TremorInfo.VorbisDSPState, AkGetNumChannels( m_VorbisState.TremorInfo.uChannelMask ) ) )
	{
		return AK_Fail;
	}
	return AK_Success;
}

/** 2012 0x975cd0. */
AKRESULT CAkSrcFileVorbis::ProcessFirstBuffer()
{
	m_bWaitForBuffering = m_pCtx->m_bRequiresPreBuffering;
	void * pBuffer;
	const AKRESULT eStmResult = m_pStream->GetBuffer( pBuffer, m_ulSizeLeft, false );
	if( eStmResult == AK_NoDataReady )
	{
		return AK_FormatNotReady;
	}
	if( eStmResult != AK_DataReady && eStmResult != AK_NoMoreData )
	{
		return AK_Fail;
	}

	AKRESULT eResult;
	if( m_VorbisState.TremorInfo.ReturnInfo.eDecoderState != UNINITIALIZED )
	{
		eResult = ProcessStreamBuffer( (AkUInt8 *)pBuffer );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	else
	{
		eResult = ParseHeader( (AkUInt8 *)pBuffer );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		m_uCurSample = 0;
		m_uLoopCnt = m_pCtx->m_LoopCount;
		eResult = ProcessStreamBuffer( (AkUInt8 *)pBuffer );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		m_pNextAddress += m_uDataOffset;
		m_ulSizeLeft -= m_uDataOffset;
	}

	eResult = DecodeVorbisHeader();
	if( eResult == AK_Success )
	{
		AkUInt32 uSrcOffsetRemainder = 0;
		if( m_pCtx->m_bSeekDirty )
		{
			eResult = SeekToSourceOffset();
			if( m_ulSizeLeft )
			{
				ReleaseStreamBuffer();
				m_pNextAddress = NULL;
				m_ulSizeLeft = 0;
			}
			uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
			m_pCtx->SetSourceOffsetRemainder( 0 );
			m_uCurSample += uSrcOffsetRemainder;
		}
		VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
		m_bFormatReady = 1;
	}
	else if( eResult == AK_FormatNotReady && !m_ulSizeLeft )
	{
		ReleaseStreamBuffer();
		m_pNextAddress = NULL;
		return AK_FormatNotReady;
	}
	return eResult;
}

/** 2012 0x975e70. */
AKRESULT CAkSrcFileVorbis::StartStream()
{
	if( m_bFormatReady )
	{
		return IsInitialPrebufferingReady();
	}
	if( m_pStream && m_VorbisState.TremorInfo.ReturnInfo.eDecoderState < PACKET_STREAM )
	{
		const AKRESULT eResult = ProcessFirstBuffer();
		if( eResult != AK_Success )
		{
			return eResult;
		}
		return IsInitialPrebufferingReady();
	}

	AkAutoStmBufSettings bufSettings;
	bufSettings.uBufferSize = 0;
	bufSettings.uMinBufferSize = 2048;
	bufSettings.uBlockSize = 0;
	AKRESULT eResult = CreateStream( bufSettings, 0 );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	bool bUsePrefetchedData;
	eResult = HandlePrefetch( bUsePrefetchedData );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AKRESULT eStartResult = m_pStream->Start();
	if( eStartResult != AK_Success )
	{
		return eStartResult;
	}
	if( bUsePrefetchedData )
	{
		m_uCurSample = 0;
		m_uLoopCnt = m_pCtx->m_LoopCount;
		eResult = DecodeVorbisHeader();
		if( eResult != AK_Success )
		{
			return eResult;
		}
		VorbisDSPRestart( 0 );
		return eResult;
	}
	eResult = ProcessFirstBuffer();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return IsInitialPrebufferingReady();
}
