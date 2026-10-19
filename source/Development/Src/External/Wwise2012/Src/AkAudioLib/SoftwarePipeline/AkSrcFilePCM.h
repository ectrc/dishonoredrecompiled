/*=============================================================================
	AkSrcFilePCM.h - CAkSrcFilePCM (aksrcfilepcm.cpp/.h): a streamed PCM source; a sample frame split across two stream
	buffers is stitched in m_pStitchBuffer.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (96 bytes, CAkVPLSrcNode::Create mallocs 0x60).
=============================================================================*/
#pragma once

#include "AkSrcFileBase.h"

class CAkSrcFilePCM : public CAkSrcFileBase
{
public:
	explicit CAkSrcFilePCM( CAkPBI * in_pCtx );

	virtual void ReleaseBuffer();
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual void StopStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT ChangeSourcePosition();

	AkUInt8 * m_pStitchBuffer;
	AkUInt32 m_uNumBytesBuffered;
	AkUInt32 m_uSizeToRelease;

protected:
	virtual AKRESULT ParseHeader( AkUInt8 * in_pBuffer );
	virtual AkReal32 GetThroughput( const AkAudioFormat & in_rFormat );
	virtual AKRESULT FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset );
	virtual AkUInt16 GetBlockAlign() const;
};
static_assert( sizeof( CAkSrcFilePCM ) == 0x60, "CAkSrcFilePCM: CAkVPLSrcNode::Create mallocs 0x60" );
