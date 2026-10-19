/*=============================================================================
	AkSrcFileADPCM.h - CAkSrcFileADPCM (aksrcfileadpcm.cpp/.h): a streamed ADPCM source; a block split across two stream
	buffers is gathered in m_ExtraBlock.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (312 bytes, CAkVPLSrcNode::Create mallocs 0x138).
=============================================================================*/
#pragma once

#include "AkSrcFileBase.h"

class CAkSrcFileADPCM : public CAkSrcFileBase
{
public:
	explicit CAkSrcFileADPCM( CAkPBI * in_pCtx );

	virtual void ReleaseBuffer();
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual void StopStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT ChangeSourcePosition();

	AkUInt32 m_uInputBlockSize;
	AkUInt8 * m_pOutBuffer;
	AkUInt8 m_ExtraBlock[ 216 ];
	AkUInt16 m_wExtraSize;

protected:
	virtual AKRESULT ParseHeader( AkUInt8 * in_pBuffer );
	virtual AkReal32 GetThroughput( const AkAudioFormat & in_rFormat );
	virtual AKRESULT FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset );
};
static_assert( sizeof( CAkSrcFileADPCM ) == 0x138, "CAkSrcFileADPCM: CAkVPLSrcNode::Create mallocs 0x138" );
