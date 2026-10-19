/*=============================================================================
	AkVPLMixBusNode.h - the bus half of the pipeline: CAkBusVolumes, CAkBusFX (akbusfx), CAkVPLMixBusNode
	(akvplmixbusnode.cpp) and CAkVPLFinalMixNode (akvplfinalmixnode.cpp). Layouts are the 2012 PDB's (mix bus node
	0x100 bytes, final mix node 0xE0).

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * CAkBusFX's four effect slots keep the PDB layout but their plug-in pointers are opaque: a bus insert effect
	    needs the effect plug-in interfaces (IH4). SetInsertFx (0x93b430) is the IH4 seam: it leaves a slot empty, so
	    ProcessAllFX and the final mix's effect loop find nothing to run.
	  * Environment volumes (CAkEnvironmentsMgr::GetEnvironmentVolumes) are IH3/IH4's; an aux (environmental) bus node
	    mixes at unity as retail does for m_EnvID 0.
=============================================================================*/
#pragma once

#include "AkMixer.h"
#include "AkBusCtx.h"

class CAkBusVolumes
{
public:
	AkReal32 m_fNextVolume;
	AkReal32 m_fPreviousVolume;
	AkReal32 m_fNextVolumedB;
};

class __declspec(align(16)) CAkBusFX : public CAkBusVolumes
{
public:
	struct FX
	{
		AkUniqueID	id;
		void *		pParam;
		void *		pEffect;
		void *		pBusFXContext;
		AkUInt8		bBypass : 1;
		AkUInt8		bLastBypass : 1;
	};

	void SetInsertFxBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	AKRESULT SetInsertFx( const AK::CAkBusCtx & in_BusContext, AkUInt32 in_uFXIndex, AkUniqueID in_EnvID );
	void DropFx( AkUInt32 in_uFXIndex );
	void DropFx();

	CAkMixer			m_Mixer;
	AkAudioBufferBus	m_BufferOut;
	AkUInt32			m_ulBufferOutSize;
	FX					m_aFX[ 4 ];
	AkUInt8				m_bBypassAllFX : 1;
	AkUInt8				m_bLastBypassAllFX : 1;

protected:
	void InitFX();
};

class __declspec(align(16)) CAkVPLMixBusNode : public CAkBusFX
{
public:
	AKRESULT Init( AkUInt32 in_uChannelMask, AkUInt16 in_uMaxFrames, AK::CAkBusCtx in_busContext );
	AKRESULT Term();
	void ReleaseBuffer();
	void Connect();
	void Disconnect() { --m_uConnectCount; }
	void ConsumeBuffer( AkAudioBufferBus & io_rBuffer );
	void ConsumeBuffer( AkVPLState & io_rVPLState, AkInt8 in_iVPLEnvironmentIndex );
	void GetResultingBuffer( AkAudioBufferBus *& io_rpBuffer );
	AKRESULT SetAllInsertFx( AkUniqueID in_EnvID );

	AK::CAkBusCtx		m_BusContext;
	VPLNodeState		m_eState;
	AkSpeakerVolumes	m_sPreviousEnvVolumes;
	AkUniqueID			m_EnvID;
	AkUInt32			m_uConnectCount;

private:
	void ProcessAllFX();
	void PostProcessFx( AkAudioBufferBus *& io_rpBuffer );
};
static_assert( sizeof( CAkVPLMixBusNode ) == 0x100, "CAkVPLMixBusNode: 2012 sizeof 0x100 (CreateEnvMixBus)" );

class __declspec(align(16)) CAkVPLFinalMixNode : public CAkBusFX
{
public:
	AKRESULT Init( AkUInt32 in_uChannelMask );
	AKRESULT Term();
	AKRESULT ReleaseBuffer();
	void Connect( CAkVPLMixBusNode * in_pInput );
	AKRESULT ConsumeBuffer( AkAudioBufferBus *& io_rpBuffer );
	void GetResultingBuffer( AkPipelineBufferBase & io_rBuffer );

	VPLNodeState m_eState;
};
static_assert( sizeof( CAkVPLFinalMixNode ) == 0xE0, "CAkVPLFinalMixNode: 2012 sizeof 0xE0 (CAkLEngine::Init)" );
