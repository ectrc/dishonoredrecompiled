/*=============================================================================
	AkSrcFileVorbis.h - CAkSrcFileVorbis (aksrcfilevorbis.cpp/.h): a streamed Wwise Vorbis source. Packets split across
	two stream buffers are stitched in m_pStitchStreamBuffer; the header packets are gathered into m_pOggPacketData.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (284 bytes, CreateVorbisFilePlugin mallocs
	0x11C). GetThroughput is the header inline: the 2012 vtable slot is a shared `fld1; ret 4` (1.0), the real stream
	throughput being set by ParseHeader from nAvgBytesPerSec.
=============================================================================*/
#pragma once

#include "AkSrcFileBase.h"
#include "AkVorbisCodec.h"

class CAkSrcFileVorbis : public CAkSrcFileBase
{
public:
	explicit CAkSrcFileVorbis( CAkPBI * in_pCtx );
	virtual ~CAkSrcFileVorbis();

	virtual void ReleaseBuffer();
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT StartStream();
	virtual void StopStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT StopLooping();
	virtual AKRESULT ChangeSourcePosition();

	AkVorbisSourceState m_VorbisState;
	AkUInt8 * m_pStitchStreamBuffer;
	AkUInt16 m_uStitchBufferEndOffset;
	AkUInt16 m_uStitchBufferValidDataSize;
	AkUInt16 m_uStitchBufferLeft;
	OggPacketHeader m_OggPacketHeader;
	AkUInt8 * m_pOggPacketData;
	AkUInt32 m_uPacketDataGathered;
	AkUInt32 m_uPacketHeaderGathered;

protected:
	virtual AKRESULT OnLoopComplete( bool in_bEndOfFile );
	virtual AKRESULT ParseHeader( AkUInt8 * in_pBuffer );
	virtual AkReal32 GetThroughput( const AkAudioFormat & in_rFormat ) { return 1.f; }
	virtual AKRESULT FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset );

private:
	AKRESULT InitVorbisInfo();
	AKRESULT DecodeVorbisHeader();
	AKRESULT ProcessFirstBuffer();
	AKRESULT GetNextPacket( ogg_packet & out_OggPacket );
	void VorbisDSPRestart( AkUInt16 in_uSrcOffsetRemainder );
	void SubmitBufferAndUpdateVorbis( AkVPLState & io_state );
	void FreeStitchBuffer();
};
static_assert( sizeof( CAkSrcFileVorbis ) == 0x11C, "CAkSrcFileVorbis: CreateVorbisFilePlugin mallocs 0x11C" );
