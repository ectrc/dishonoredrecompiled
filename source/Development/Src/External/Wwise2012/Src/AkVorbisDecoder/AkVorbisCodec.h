/*=============================================================================
	AkVorbisCodec.h - what the Vorbis sources (CAkSrcBankVorbis, CAkSrcFileVorbis) share: the 'fmt ' chunk of a Wwise Vorbis
	file, the decoder state, the packet reader and DecodeVorbis / VorbisSeek (akvorbiscodec.h / akvorbiscodec.cpp).

	DISHONORED(port): agent IH2 (wave 26). Layouts are the 2012 PDB's. The 'fmt ' chunk of a Vorbis media item is
	WaveFormatVorbis = WaveFormatExtensible (24 bytes) + AkVorbisHeader (dwTotalPCMFrames + AkVorbisInfo) = 66 bytes, packed
	to 2 as the PDB records (__unaligned align(2)); measured on the voice line 0x5c7ca16c (fmt chunk size 66). The data chunk
	is: the seek table (dwSeekTableSize bytes of AkVorbisSeekTableItem), the setup packet, then the audio packets, every
	packet prefixed by its 16-bit size (OggPacketHeader).
=============================================================================*/
#pragma once

#include "codec_internal.h"

class CAkPBI;

enum AkVorbisDecoderState
{
	UNINITIALIZED = 0,
	INITIALIZED = 1,
	SEEKTABLEINTIALIZED = 2,
	PACKET_STREAM = 3,
	END_OF_STREAM = 4
};

struct AkTremorInfoReturn
{
	AkUInt32 uFramesProduced;
	AKRESULT eDecoderStatus;
	AkVorbisDecoderState eDecoderState;
	AkUInt32 uInputBytesConsumed;
};

struct AkTremorInfo
{
	AkTremorInfoReturn ReturnInfo;
	vorbis_dsp_state VorbisDSPState;
	AkUInt8 * pucData;
	AkUInt32 uChannelMask;
	AkUInt32 uRequestedFrames;
	AkUInt32 uInputDataSize;
	bool bNoMoreInputPackets;
};

struct AkVorbisLoopInfo
{
	AkUInt32 dwLoopStartPacketOffset;
	AkUInt32 dwLoopEndPacketOffset;
	AkUInt16 uLoopBeginExtra;
	AkUInt16 uLoopEndExtra;
};

struct AkVorbisSeekTableItem
{
	AkUInt16 uPacketFrameOffset;
	AkUInt16 uPacketFileOffset;
};

struct OggPacketHeader
{
	AkUInt16 uPacketSize;
};

#pragma pack( push, 2 )

struct AkVorbisInfo
{
	AkVorbisLoopInfo LoopInfo;
	AkUInt32 dwSeekTableSize;
	AkUInt32 dwVorbisDataOffset;
	AkUInt16 uMaxPacketSize;
	AkUInt16 uLastGranuleExtra;
	AkUInt32 dwDecodeAllocSize;
	AkUInt32 dwDecodeX64AllocSize;
	AkUInt32 uHashCodebook;
	AkUInt8 uBlockSizes[ 2 ];
};

struct AkVorbisHeaderBase
{
	AkUInt32 dwTotalPCMFrames;
};

struct AkVorbisHeader : public AkVorbisHeaderBase, public AkVorbisInfo
{
};

struct WaveFormatEx
{
	AkUInt16 wFormatTag;
	AkUInt16 nChannels;
	AkUInt32 nSamplesPerSec;
	AkUInt32 nAvgBytesPerSec;
	AkUInt16 nBlockAlign;
	AkUInt16 wBitsPerSample;
	AkUInt16 cbSize;
};

struct WaveFormatExtensible : public WaveFormatEx
{
	AkUInt16 wSamplesPerBlock;
	AkUInt32 dwChannelMask;
};

struct WaveFormatVorbis : public WaveFormatExtensible
{
	AkVorbisHeader vorbisHeader;
};

#pragma pack( pop )

struct AkVorbisSourceState
{
	AkTremorInfo TremorInfo;
	AkVorbisInfo VorbisInfo;
	AkUInt32 uSampleRate;
	AkVorbisSeekTableItem * pSeekTable;
	AkUInt32 uSeekTableSizeRead;
};

static_assert( sizeof( AkVorbisInfo ) == 38, "AkVorbisInfo: 38 bytes, packed to 2 (2012 PDB)" );
static_assert( sizeof( WaveFormatExtensible ) == 24, "WaveFormatExtensible: 24 bytes (2012 PDB)" );
static_assert( sizeof( WaveFormatVorbis ) == 66, "WaveFormatVorbis: the 66-byte 'fmt ' chunk of a Vorbis media item" );
static_assert( sizeof( AkTremorInfo ) == 124, "AkTremorInfo: 2012 PDB sizeof 124" );

class AkVorbisPacketReader
{
public:
	AKRESULT ReadPacket( AkVorbisDecoderState & io_eState, AkUInt16 in_uMaxPacketSize, bool in_bNoMoreInputPackets, ogg_packet & out_Packet );

	AkUInt8 * m_pFirstPacketStart;
	AkUInt32 m_uCurrentOffset;
	AkUInt32 m_uTotalSize;
};

void DecodeVorbis( AkTremorInfo * in_pTremorInfo, AkUInt16 in_uMaxPacketSize, AkUInt8 * in_pInputBuf, AkInt16 * in_pOuputBuf );
AKRESULT VorbisSeek( const AkVorbisSourceState & in_vorbisState, AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset );
