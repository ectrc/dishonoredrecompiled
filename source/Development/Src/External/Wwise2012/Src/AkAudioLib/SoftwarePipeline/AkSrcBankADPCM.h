/*=============================================================================
	AkSrcBankADPCM.h - CAkSrcBankADPCM (aksrcbankadpcm.cpp/.h): an ADPCM source decoded from in-memory media.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (64 bytes, CAkVPLSrcNode::Create mallocs 0x40).
=============================================================================*/
#pragma once

#include "AkSrcBase.h"

class CAkSrcBankADPCM : public CAkSrcBaseEx
{
public:
	explicit CAkSrcBankADPCM( CAkPBI * in_pCtx );

	virtual void ReleaseBuffer();
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT StartStream();
	virtual void StopStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT ChangeSourcePosition();

	AkUInt8 * m_pucData;
	AkUInt16 m_uInputBlockSize;
	AkUInt8 * m_pOutBuffer;

protected:
	virtual AKRESULT OnLoopComplete( bool in_bEndOfFile );

private:
	AKRESULT SeekToSourceOffset();
};
static_assert( sizeof( CAkSrcBankADPCM ) == 0x40, "CAkSrcBankADPCM: CAkVPLSrcNode::Create mallocs 0x40" );
