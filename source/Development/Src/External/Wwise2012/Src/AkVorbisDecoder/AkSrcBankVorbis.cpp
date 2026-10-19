/*=============================================================================
	AkSrcBankVorbis.cpp - CAkSrcBankVorbis and CreateVorbisBankPlugin (aksrcbankvorbis.cpp, 2012 0x976040..0x976640;
	2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). The media's data chunk is the seek table, the setup packet (shared
	through g_VorbisCodebookMgr) and the audio packets; DecodeVorbis writes up to 1024 interleaved 16-bit frames into a
	CAkLEngine cached buffer, which is the buffer handed down the pipeline.
=============================================================================*/
#include "AkSrcBankVorbis.h"
#include "AkVorbisCodebookMgr.h"
#include "AkFileParser.h"
#include "AkLEngine.h"
#include "AkPBI.h"
#include <AK/Plugin/AkVorbisFactory.h>

/** 2012 0x9765e0. */
IAkSoftwareCodec * CreateVorbisBankPlugin( void * in_pCtx )
{
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcBankVorbis ) );
	return pMem ? AkPlacementNew( pMem ) CAkSrcBankVorbis( (CAkPBI *)in_pCtx ) : NULL;
}

/** 2012 0x9762b0 (the deleting destructor; no rva of its own, read from the shipping executable's bytes): the voice
	drops its reference on the shared setup header. */
CAkSrcBankVorbis::~CAkSrcBankVorbis()
{
	if( m_VorbisState.TremorInfo.VorbisDSPState.csi )
	{
		g_VorbisCodebookMgr.ReleaseCodebook( m_VorbisState );
	}
}

/** 2012 0x976040. */
void CAkSrcBankVorbis::VorbisDSPRestart( AkUInt16 in_uSrcOffsetRemainder )
{
	const AkUInt16 uExtraSamplesEnd = ( m_uLoopCnt == 1 ) ? m_VorbisState.VorbisInfo.uLastGranuleExtra : m_VorbisState.VorbisInfo.LoopInfo.uLoopEndExtra;
	vorbis_dsp_restart( &m_VorbisState.TremorInfo.VorbisDSPState, in_uSrcOffsetRemainder, uExtraSamplesEnd );
	m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = PACKET_STREAM;
}

/** 2012 0x976080. */
void CAkSrcBankVorbis::ReleaseBuffer()
{
	if( m_VorbisState.TremorInfo.pucData )
	{
		CAkLEngine::ReleaseCachedAudioBuffer( AkGetNumChannels( m_VorbisState.TremorInfo.uChannelMask ) * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkInt16 ), m_VorbisState.TremorInfo.pucData );
		m_VorbisState.TremorInfo.ReturnInfo.uFramesProduced = 0;
		m_VorbisState.TremorInfo.pucData = NULL;
	}
}

/** 2012 0x9760d0: the current loop ends on the file's last granule. */
AKRESULT CAkSrcBankVorbis::StopLooping()
{
	m_VorbisState.TremorInfo.VorbisDSPState.state.extra_samples_end = m_VorbisState.VorbisInfo.uLastGranuleExtra;
	return CAkSrcBaseEx::StopLooping();
}

/** 2012 0x9760f0: the seek table's packet at or before the sample. */
AKRESULT CAkSrcBankVorbis::VirtualSeek( AkUInt32 & io_uSeekPosition )
{
	AkUInt32 uFileOffset;
	if( VorbisSeek( m_VorbisState, io_uSeekPosition, io_uSeekPosition, uFileOffset ) != AK_Success )
	{
		return AK_Fail;
	}
	m_pucData = m_pucDataStart + uFileOffset;
	return AK_Success;
}

/** 2012 0x976140. */
AKRESULT CAkSrcBankVorbis::SeekToNativeOffset()
{
	if( !m_VorbisState.pSeekTable )
	{
		return AK_Fail;
	}
	const AkUInt32 uSourceOffset = GetSourceOffset();
	if( uSourceOffset >= m_uTotalSamples )
	{
		return AK_Fail;
	}
	AkUInt32 uRealOffset = uSourceOffset;
	if( VirtualSeek( uRealOffset ) != AK_Success )
	{
		return AK_Fail;
	}
	m_uCurSample = uRealOffset;
	m_pCtx->SetSourceOffsetRemainder( uSourceOffset - uRealOffset );
	return AK_Success;
}

/** 2012 0x9761b0: the decoder skips the remainder between the seek table packet and the requested sample. */
AKRESULT CAkSrcBankVorbis::ChangeSourcePosition()
{
	const AKRESULT eResult = SeekToNativeOffset();
	const AkUInt32 uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
	m_pCtx->SetSourceOffsetRemainder( 0 );
	m_uCurSample += uSrcOffsetRemainder;
	VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
	return eResult;
}

/** 2012 0x976220. */
AKRESULT CAkSrcBankVorbis::OnLoopComplete( bool in_bEndOfFile )
{
	if( m_uLoopCnt > 1 )
	{
		--m_uLoopCnt;
	}
	if( !in_bEndOfFile )
	{
		m_pucData = m_pucDataStart + m_VorbisState.VorbisInfo.dwSeekTableSize + m_VorbisState.VorbisInfo.LoopInfo.dwLoopStartPacketOffset;
		VorbisDSPRestart( m_VorbisState.VorbisInfo.LoopInfo.uLoopBeginExtra );
		m_VorbisState.TremorInfo.ReturnInfo.eDecoderStatus = AK_DataReady;
	}
	return in_bEndOfFile ? AK_NoMoreData : AK_DataReady;
}

/** 2012 0x9762f0: decodes up to the end of the data, or of the loop's last packet while loops remain. */
void CAkSrcBankVorbis::GetBuffer( AkVPLState & io_state )
{
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
	m_VorbisState.TremorInfo.uRequestedFrames = io_state.buffer.uMaxFrames;
	const AkUInt8 * pEndOfInput = ( m_uLoopCnt == 1 )
		? m_pucDataStart + m_uDataSize
		: m_pucDataStart + m_VorbisState.VorbisInfo.dwSeekTableSize + m_VorbisState.VorbisInfo.LoopInfo.dwLoopEndPacketOffset;
	m_VorbisState.TremorInfo.uInputDataSize = (AkUInt32)( pEndOfInput - m_pucData );
	m_VorbisState.TremorInfo.bNoMoreInputPackets = true;
	DecodeVorbis( &m_VorbisState.TremorInfo, m_VorbisState.VorbisInfo.uMaxPacketSize, m_pucData, (AkInt16 *)m_VorbisState.TremorInfo.pucData );
	io_state.result = m_VorbisState.TremorInfo.ReturnInfo.eDecoderStatus;
	if( io_state.result == AK_Fail )
	{
		return;
	}
	m_pucData += m_VorbisState.TremorInfo.ReturnInfo.uInputBytesConsumed;
	SubmitBufferAndUpdate( m_VorbisState.TremorInfo.pucData, (AkUInt16)m_VorbisState.TremorInfo.ReturnInfo.uFramesProduced, m_VorbisState.uSampleRate, m_VorbisState.TremorInfo.uChannelMask, io_state );
}

/** 2012 0x9763d0. */
void CAkSrcBankVorbis::StopStream()
{
	vorbis_dsp_clear( &m_VorbisState.TremorInfo.VorbisDSPState );
	ReleaseBuffer();
	if( m_VorbisState.pSeekTable )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_VorbisState.pSeekTable );
		m_VorbisState.pSeekTable = NULL;
	}
	CAkSrcBaseEx::StopStream();
}

/** 2012 0x976420. */
AKRESULT CAkSrcBankVorbis::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	AKRESULT eResult = AK_Success;
	AkUInt32 uSrcOffsetRemainder = 0;
	if( eBehavior == AkVirtualQueueBehavior_FromElapsedTime )
	{
		if( in_bUseSourceOffset )
		{
			eResult = SeekToNativeOffset();
		}
		else
		{
			eResult = VirtualSeek( m_uCurSample );
			if( eResult != AK_Success )
			{
				m_uCurSample = 0;
				eResult = VirtualSeek( m_uCurSample );
			}
		}
		uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
		m_pCtx->SetSourceOffsetRemainder( 0 );
		m_uCurSample += uSrcOffsetRemainder;
	}
	else if( eBehavior == AkVirtualQueueBehavior_FromBeginning )
	{
		m_pucData = m_pucDataStart + m_VorbisState.VorbisInfo.dwVorbisDataOffset;
		m_uCurSample = 0;
		m_uLoopCnt = m_pCtx->m_LoopCount;
	}
	else
	{
		return eResult;
	}
	VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
	return eResult;
}

/** 2012 0x9764f0: copies the seek table out of the media and decodes (or shares) the setup packet. */
AKRESULT CAkSrcBankVorbis::DecodeVorbisHeader()
{
	const AkUInt32 dwSeekTableSize = m_VorbisState.VorbisInfo.dwSeekTableSize;
	if( dwSeekTableSize )
	{
		m_VorbisState.pSeekTable = (AkVorbisSeekTableItem *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, dwSeekTableSize );
		if( !m_VorbisState.pSeekTable )
		{
			return AK_InsufficientMemory;
		}
	}
	m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = INITIALIZED;
	if( dwSeekTableSize )
	{
		memcpy( m_VorbisState.pSeekTable, m_pucData, dwSeekTableSize );
		m_pucData += dwSeekTableSize;
	}

	ogg_packet Packet;
	Packet.buffer.size = *(AkUInt16 *)m_pucData;
	Packet.buffer.data = m_pucData + sizeof( OggPacketHeader );
	Packet.e_o_s = 0;
	m_pucData += sizeof( OggPacketHeader ) + Packet.buffer.size;
	CAkVorbisAllocator * pAllocator = g_VorbisCodebookMgr.Decodebook( m_VorbisState, m_pCtx, &Packet );
	if( !pAllocator )
	{
		return AK_Fail;
	}
	m_VorbisState.TremorInfo.VorbisDSPState.csi = (codec_setup_info *)pAllocator->pStartAddress;
	if( vorbis_dsp_init( &m_VorbisState.TremorInfo.VorbisDSPState, AkGetNumChannels( m_VorbisState.TremorInfo.uChannelMask ) ) )
	{
		return AK_Fail;
	}
	m_VorbisState.TremorInfo.ReturnInfo.eDecoderState = PACKET_STREAM;
	return AK_Success;
}

/** 2012 0x976640. */
AKRESULT CAkSrcBankVorbis::StartStream()
{
	AkUInt8 * pDataPtr = m_pCtx->m_pDataPtr;
	const AkUInt32 ulBufferSize = m_pCtx->m_uDataSize;
	if( !pDataPtr )
	{
		return AK_Fail;
	}
	AkFileParser::FormatInfo fmtInfo;
	AKRESULT eResult = AkFileParser::Parse( pDataPtr, ulBufferSize, fmtInfo, &m_markers, &m_uPCMLoopStart, &m_uPCMLoopEnd, &m_uDataSize, &m_uDataOffset, NULL );
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

	m_pucDataStart = pDataPtr + m_uDataOffset;
	m_uTotalSamples = pFmt->vorbisHeader.dwTotalPCMFrames;
	m_VorbisState.VorbisInfo = pFmt->vorbisHeader;
	m_VorbisState.uSampleRate = pFmt->nSamplesPerSec;
	m_VorbisState.TremorInfo.uChannelMask = pFmt->dwChannelMask;
	if( !m_uPCMLoopEnd )
	{
		m_uPCMLoopEnd = m_uTotalSamples - 1;
	}
	if( m_uPCMLoopEnd < m_uPCMLoopStart || m_uPCMLoopEnd >= m_uTotalSamples || ulBufferSize != m_uDataOffset + m_uDataSize )
	{
		return AK_Fail;
	}
	m_pucData = m_pucDataStart;
	m_uCurSample = 0;
	m_uLoopCnt = m_pCtx->m_LoopCount;
	eResult = DecodeVorbisHeader();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	AkUInt32 uSrcOffsetRemainder = 0;
	if( m_pCtx->m_bSeekDirty )
	{
		eResult = SeekToNativeOffset();
		uSrcOffsetRemainder = m_pCtx->GetSourceOffsetRemainder();
		m_pCtx->SetSourceOffsetRemainder( 0 );
		m_uCurSample += uSrcOffsetRemainder;
	}
	VorbisDSPRestart( (AkUInt16)uSrcOffsetRemainder );
	return eResult;
}
