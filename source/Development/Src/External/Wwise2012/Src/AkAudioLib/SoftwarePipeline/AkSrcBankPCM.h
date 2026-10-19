/*=============================================================================
	AkSrcBankPCM.h - CAkSrcBankPCM (aksrcbankpcm.cpp/.h): a PCM source played straight from in-memory media.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (56 bytes, CAkVPLSrcNode::Create mallocs 0x38).
=============================================================================*/
#pragma once

#include "AkSrcBase.h"

class CAkSrcBankPCM : public CAkSrcBaseEx
{
public:
	explicit CAkSrcBankPCM( CAkPBI * in_pCtx );

	virtual void ReleaseBuffer() {}
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT StartStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT ChangeSourcePosition();

	AkUInt8 * m_pDataStart;
};
static_assert( sizeof( CAkSrcBankPCM ) == 0x38, "CAkSrcBankPCM: CAkVPLSrcNode::Create mallocs 0x38" );
