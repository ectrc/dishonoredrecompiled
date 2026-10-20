/*=============================================================================
	AkFDNReverbFX.h - CAkFDNReverbFX, the Matrix Reverb (Wwise 2012.1, plugins/effects/akfdnreverb/win32/akfdnreverbfx.h).
	Agent IH4 (fork F2). 480 bytes, 16-aligned, as the 2012 PDB. Up to 16 delay lines in groups of four, each group's four
	lines interleaved in one buffer (m_pfDelayStart[g], stride 4); m_fpPerformDSP is the routine InitDelayLines picks for
	the channel configuration and the delay count.
=============================================================================*/
#ifndef _AK_FDNREVERBFX_H_
#define _AK_FDNREVERBFX_H_

#include "AkFDNReverbFXParams.h"
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>
#include <xmmintrin.h>

class CAkFDNReverbFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkFDNReverbFX();
	~CAkFDNReverbFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	typedef void ( CAkFDNReverbFX::*PerformDSPFunc )( AkAudioBuffer * io_pBuffer );

	AKRESULT InitDelayLines( AkChannelMask in_uChannelMask );
	void TermDelayLines();
	void ComputeIIRLPFCoefs();
	void MakePrimeNumber( AkUInt32 & in_uIn );

	template < AkUInt32 NUMGROUPS, AkUInt32 CONFIG > void ProcessFDN( AkAudioBuffer * io_pBuffer );

	void ProcessMono4( AkAudioBuffer * io_pBuffer );
	void ProcessMono8( AkAudioBuffer * io_pBuffer );
	void ProcessMono12( AkAudioBuffer * io_pBuffer );
	void ProcessMono16( AkAudioBuffer * io_pBuffer );
	void ProcessStereo4( AkAudioBuffer * io_pBuffer );
	void ProcessStereo8( AkAudioBuffer * io_pBuffer );
	void ProcessStereo12( AkAudioBuffer * io_pBuffer );
	void ProcessStereo16( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointZero4( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointZero8( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointZero12( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointZero16( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointOne4( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointOne8( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointOne12( AkAudioBuffer * io_pBuffer );
	void ProcessFivePointOne16( AkAudioBuffer * io_pBuffer );
	void ProcessN4( AkAudioBuffer * io_pBuffer );
	void ProcessN8( AkAudioBuffer * io_pBuffer );
	void ProcessN12( AkAudioBuffer * io_pBuffer );
	void ProcessN16( AkAudioBuffer * io_pBuffer );

	PerformDSPFunc				m_fpPerformDSP;
	CAkFDNReverbFXParams *		m_pParams;
	AK::IAkPluginMemAlloc *		m_pAllocator;
	AkReal32					m_fCachedReverbTime;
	AkReal32					m_fCachedHFRatio;
	AkReal32 *					m_pfPreDelayStart;
	AkReal32 *					m_pfPreDelayRW;
	AkReal32 *					m_pfPreDelayEnd;
	AkUInt32					m_uPreDelayLength;
	AkReal32					m_fFIRLPFB0;
	AkReal32					m_fFIRLPFB1;
	AkReal32					m_fFIRLPFMem;
	AkUInt32					m_uNominalDelayLength[ 16 ];
	AkReal32 *					m_pfDelayRead[ 16 ];
	AkReal32 *					m_pfDelayStart[ 4 ];
	AkReal32 *					m_pfDelayWrite[ 4 ];
	AkReal32 *					m_pfDelayEnd[ 4 ];
	__m128						m_vIIRLPFB0[ 4 ];
	__m128						m_vIIRLPFA1[ 4 ];
	__m128						m_vIIRLPFMem[ 4 ];
	AkReal32					m_fDCFwdMem;
	AkReal32					m_fDCFbkMem;
	AkReal32					m_fDCCoef;
	AkReal32					m_fCurrentDry;
	AkReal32					m_fCurrentWet;
	AkFXTailHandler				m_FXTailHandler;
	AkUInt32					m_uSampleRate;
	AkUInt32					m_uTailLength;
	AkUInt32					m_uNumProcessedChannels;
	bool						m_bIsSentMode;
	bool						m_bPrevPreStop;
};

static_assert( sizeof( CAkFDNReverbFX ) == 480, "CAkFDNReverbFX: 2012 PDB sizeof 480" );

#endif
