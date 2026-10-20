/*=============================================================================
	AkVPLSrcCbxNode.h - one voice's pipeline: CAkVPLSrcCbxNode (akvplsrccbxnode.cpp) holds the source (and the next one
	of a sample-accurate sequence), the pitch node (akvplpitchnode.cpp), the up-to-four insert effects
	(akvplfilternode*.cpp), the voice low-pass filter and the obstruction low-pass filter (akvpllpfnode.cpp).

	DISHONORED(port): agent IH2 (wave 26). Layouts are the 2012 PDB's (CAkVPLSrcCbxNode 0x288 bytes).
	The four insert-effect slots hold CAkVPLFilterNode / CAkVPLFilterNodeOutOfPlace (AkVPLFilterNode.h, agent IH4).
=============================================================================*/
#pragma once

#include "AkVPLNode.h"
#include "AkResampler.h"
#include "AkSrcLpFilter.h"
#include "AkVPLFilterNode.h"

class CAkVPLSrcCbxNode;

class CAkVPLPitchNode : public CAkVPLNode
{
public:
	explicit CAkVPLPitchNode( CAkVPLSrcCbxNode * in_pCbx ) : m_pCbx( in_pCbx ), m_pPBI( NULL ), m_bLast( false ), m_bStartPosInfoUpdated( false ), m_bPadFrameOffset( true ) {}

	virtual void ReleaseBuffer();
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT Seek();

	void Init( AkAudioFormat * io_pFormat, CAkPBI * in_pPBI, AkUInt32 in_usSampleRate );
	AKRESULT Term();
	void ResetPadFrameOffset() { m_bPadFrameOffset = true; }
	void GetBuffer( AkVPLState & io_state );
	void ConsumeBuffer( AkVPLState & io_state );

	CAkResampler		m_Pitch;
	AkPipelineBuffer	m_BufferIn;
	AkPipelineBuffer	m_BufferOut;
	CAkVPLSrcCbxNode *	m_pCbx;
	CAkPBI *			m_pPBI;
	bool				m_bLast;
	bool				m_bStartPosInfoUpdated;
	bool				m_bPadFrameOffset;

protected:
	AKRESULT SwitchToNextSrc();
	void ReleaseInputBuffer( AkPipelineBuffer & io_buffer );
};

class CAkVPLLPFNode : public CAkVPLNode
{
public:
	virtual void ReleaseBuffer();
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset ) { return m_pInput ? m_pInput->VirtualOff( eBehavior, in_bUseSourceOffset ) : AK_Success; }

	void ConsumeBuffer( AkVPLState & io_state );

	CAkSrcLpFilter m_LPF;
};

struct AkVPLSrcCbxRec
{
	explicit AkVPLSrcCbxRec( CAkVPLSrcCbxNode * in_pCbx ) : m_Pitch( in_pCbx )
	{
		m_pFilter[ 0 ] = m_pFilter[ 1 ] = m_pFilter[ 2 ] = m_pFilter[ 3 ] = NULL;
	}

	void ClearVPL();

	CAkVPLPitchNode			m_Pitch;
	CAkVPLLPFNode			m_LPF;
	CAkVPLFilterNodeBase *	m_pFilter[ 4 ];
};

class CAkVPLSrcCbxNodeBase
{
public:
	CAkVPLSrcCbxNodeBase();

	CAkPBI * GetContext() const { return m_pSources[ 0 ] ? m_pSources[ 0 ]->m_pCtx : NULL; }

	CAkVPLSrcCbxNode *			pNextItem;
	CAkVPLSrcNode *				m_pSources[ 2 ];
	VPLNodeState				m_eState;
	AkVirtualQueueBehavior		m_eVirtualBehavior;
	AkBelowThresholdBehavior	m_eBelowThresholdBehavior;
	bool						m_bAudible;
	bool						m_bPreviousSilent;
	bool						m_bFirstBufferProcessed;
	AkSpeakerVolumes			m_PreviousDirect[ AK_VOICE_MAX_NUM_CHANNELS ];
	AkSpeakerVolumes			m_PreviousEnv[ AK_VOICE_MAX_NUM_CHANNELS ];
	AkEnvironmentValue			m_LastEnvironmentValues[ 4 ];
};

struct AkVoiceOutputDiag
{
	AkUInt32	uRunBuffers;
	AkUInt32	uSilentBuffers;
	AkUInt32	uSkippedBuffers;
	AkUInt32	uFrames;
	AkReal64	fSourceEnergy;
	AkReal64	fOutputEnergy;
	AkReal32	fMaxGain;
};

class CAkVPLSrcCbxNode : public CAkVPLSrcCbxNodeBase
{
public:
	CAkVPLSrcCbxNode();

	void Init( AkUInt32 in_uSampleRate ) { m_uSampleRate = in_uSampleRate; }
	void Term();
	void Start();
	void Stop();
	void Pause();
	void Resume();
	void StopLooping( CAkPBI * in_pCtx );
	void SeekSource();
	void ConsumeBuffer( AkVPLState & io_state );
	void ReleaseBuffer();
	bool StartRun( AkVPLState & io_state );
	AKRESULT AddSrc( CAkPBI * in_pCtx, bool in_bActive );
	AKRESULT AddSrc( CAkVPLSrcNode * in_pSrc, bool in_bActive );
	AKRESULT AddPipeline( bool in_bIsEnv );
	void RemovePipeline( AkCtxDestroyReason in_eReason );
	void SwitchToNextSrc();
	AKRESULT FetchStreamedData( CAkPBI * in_pCtx );
	void RestorePreviousVolumes( AkAudioBufferCbx * io_pBuffer );
	void SetLastEnvironmentalValues( const AkEnvironmentValue * in_pValues );
	bool IsUsingThisSlot( const CAkUsageSlot * in_pUsageSlot );
	void SetFxBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	void RefreshBypassFx();
	void RefreshBypassFx( AkUInt32 in_uFXIndex );
	void UpdateFx( AkUInt32 in_uFXIndex );
	void DiagRun( const AkVPLState & in_state );
	void DiagSkip();
	void DiagReport( const wchar_t * in_szWhen );

	AkUInt32		m_uSampleRate;
	AkUInt32		m_uChannelMask;
	AkVPLSrcCbxRec	m_cbxRec;
	CAkVPLLPFNode	m_ObstructionLPF;
	AkVoiceOutputDiag	m_Diag;

private:
	void GetVolumes( bool in_bEnvBus, CAkPBI * in_pContext, AkAudioBufferCbx * io_pBuffer, bool & out_bNextSilent );
	AKRESULT SourceTimeSkip( AkUInt32 in_uMaxFrames );
	void SetAudible( bool in_bAudible );
};

/** The volume thresholds AK::SoundEngine::SetVolumeThreshold sets (2012 .data 0xe36b2c / 0xe36b30: 2^-16 and -96.3 dB). */
extern AkReal32 g_fVolumeThreshold;
extern AkReal32 g_fVolumeThresholdDB;
