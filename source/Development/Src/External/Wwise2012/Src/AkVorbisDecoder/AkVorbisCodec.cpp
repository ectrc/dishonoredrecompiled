/*=============================================================================
	AkVorbisCodec.cpp - packet reading, decoding to interleaved 16-bit PCM and seek-table lookup shared by the Vorbis sources
	(akvorbiscodec.cpp / akvorbiscodec.h of AkVorbisDecoder).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail return codes: AK_DataReady (45) for a packet,
	AK_NoMoreData-on-input as AK_DataNeeded (43), AK_Fail (2) for a packet larger than uMaxPacketSize; DecodeVorbis reports
	AK_NoDataReady (46) / AK_DataReady (45) / AK_NoMoreData (17) / AK_Fail in ReturnInfo.eDecoderStatus.
=============================================================================*/
#include "AkVorbisCodec.h"

/** 2012 0x976ea0; 2013 0x973680. */
AKRESULT AkVorbisPacketReader::ReadPacket( AkVorbisDecoderState & io_eState, AkUInt16 in_uMaxPacketSize, bool in_bNoMoreInputPackets, ogg_packet & out_Packet )
{
	if( m_uCurrentOffset + sizeof( OggPacketHeader ) > m_uTotalSize )
	{
		return AK_DataNeeded;
	}
	AkUInt8 * pPacket = m_pFirstPacketStart + m_uCurrentOffset;
	const AkUInt16 uPacketSize = ( (OggPacketHeader *)pPacket )->uPacketSize;
	if( uPacketSize > in_uMaxPacketSize )
	{
		return AK_Fail;
	}
	if( io_eState == END_OF_STREAM )
	{
		return AK_DataNeeded;
	}
	const AkUInt32 uNextOffset = m_uCurrentOffset + sizeof( OggPacketHeader ) + uPacketSize;
	if( uNextOffset > m_uTotalSize )
	{
		return AK_DataNeeded;
	}
	m_uCurrentOffset = uNextOffset;
	out_Packet.buffer.data = pPacket + sizeof( OggPacketHeader );
	out_Packet.buffer.size = uPacketSize;
	out_Packet.e_o_s = m_uCurrentOffset == m_uTotalSize && in_bNoMoreInputPackets;
	if( out_Packet.e_o_s )
	{
		io_eState = END_OF_STREAM;
	}
	return AK_DataReady;
}

/** 2012 0x976f30; 2013 0x973710. */
void DecodeVorbis( AkTremorInfo * in_pTremorInfo, AkUInt16 in_uMaxPacketSize, AkUInt8 * in_pInputBuf, AkInt16 * in_pOuputBuf )
{
	AkVorbisPacketReader PacketReader;
	PacketReader.m_pFirstPacketStart = in_pInputBuf;
	PacketReader.m_uCurrentOffset = 0;
	PacketReader.m_uTotalSize = in_pTremorInfo->uInputDataSize;
	AkUInt32 uFramesLeft = in_pTremorInfo->uRequestedFrames;
	vorbis_dsp_state * pDSPState = &in_pTremorInfo->VorbisDSPState;
	do
	{
		const int iFrames = vorbis_dsp_pcmout( pDSPState, in_pOuputBuf, uFramesLeft );
		if( iFrames )
		{
			in_pOuputBuf += iFrames * pDSPState->channels;
			uFramesLeft -= iFrames;
			continue;
		}
		ogg_packet Packet;
		const AKRESULT eResult = PacketReader.ReadPacket( in_pTremorInfo->ReturnInfo.eDecoderState, in_uMaxPacketSize, in_pTremorInfo->bNoMoreInputPackets, Packet );
		if( eResult == AK_Fail )
		{
			in_pTremorInfo->ReturnInfo.uFramesProduced = 0;
			in_pTremorInfo->ReturnInfo.eDecoderStatus = AK_Fail;
			return;
		}
		if( eResult == AK_DataNeeded )
		{
			break;
		}
		vorbis_dsp_synthesis( pDSPState, &Packet );
	}
	while( uFramesLeft );

	in_pTremorInfo->ReturnInfo.uFramesProduced = in_pTremorInfo->uRequestedFrames - uFramesLeft;
	in_pTremorInfo->ReturnInfo.uInputBytesConsumed = PacketReader.m_uCurrentOffset;
	if( in_pTremorInfo->ReturnInfo.eDecoderState == END_OF_STREAM && !vorbis_dsp_pcmout( pDSPState, NULL, 0 ) )
	{
		in_pTremorInfo->ReturnInfo.eDecoderStatus = AK_NoMoreData;
	}
	else
	{
		in_pTremorInfo->ReturnInfo.eDecoderStatus = in_pTremorInfo->ReturnInfo.uFramesProduced ? AK_DataReady : AK_NoDataReady;
	}
}

/** 2012 0x975f80; 2013 0x9718a0. */
AKRESULT VorbisSeek( const AkVorbisSourceState & in_vorbisState, AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset )
{
	if( !in_uDesiredSample )
	{
		out_uSeekedSample = 0;
		out_uFileOffset = in_vorbisState.VorbisInfo.dwVorbisDataOffset;
		return AK_Success;
	}
	const AkVorbisSeekTableItem * pSeekTable = in_vorbisState.pSeekTable;
	const AkUInt32 uNumSeekTableItems = in_vorbisState.VorbisInfo.dwSeekTableSize / sizeof( AkVorbisSeekTableItem );
	if( !pSeekTable || !uNumSeekTableItems )
	{
		return AK_Fail;
	}
	AkUInt32 uFileOffset = 0;
	AkUInt32 uSample = 0;
	AkUInt32 uIndex;
	for( uIndex = 0; uIndex < uNumSeekTableItems; ++uIndex )
	{
		const AkUInt32 uNextSample = uSample + pSeekTable->uPacketFrameOffset;
		if( uNextSample > in_uDesiredSample )
		{
			break;
		}
		uFileOffset += pSeekTable->uPacketFileOffset;
		++pSeekTable;
		uSample = uNextSample;
	}
	if( uIndex )
	{
		out_uFileOffset = uFileOffset + in_vorbisState.VorbisInfo.dwSeekTableSize;
		out_uSeekedSample = uSample;
	}
	else
	{
		out_uFileOffset = in_vorbisState.VorbisInfo.dwVorbisDataOffset;
		out_uSeekedSample = 0;
	}
	return AK_Success;
}
