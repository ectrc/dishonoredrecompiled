/*=============================================================================
	AkDelayFX.h - CAkDelayFX (Wwise 2012.1, samples/plugins/akdelay/sources/audioenginefx/akdelayfx.h). Agent IH4 (fork F2).
	96 bytes as the 2012 PDB.
=============================================================================*/
#ifndef _AK_DELAYFX_H_
#define _AK_DELAYFX_H_

#include "AkDelayFXDSP.h"

class CAkDelayFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkDelayFX();
	~CAkDelayFX() {}

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	CAkDelayFXDSP			m_FXState;
	CAkDelayFXParams *		m_pParams;
	AK::IAkPluginMemAlloc *	m_pAllocator;
};

static_assert( sizeof( CAkDelayFX ) == 96, "CAkDelayFX: 2012 PDB sizeof 96" );

#endif
