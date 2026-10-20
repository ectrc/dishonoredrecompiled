/*=============================================================================
	AkLEngine.h - CAkLEngine, the lower engine (aklengine.cpp Win32 + aklengine_softwarepipeline.cpp): one VPL (voice
	list + mix bus node) per mixing bus, the final mix, the cached audio buffers and the sink.

	DISHONORED(port): agent IH2 (wave 26). Layouts are the 2012 PDB's (AkVPL 0x120 bytes). Deviations named in
	AkLEngine.cpp.
=============================================================================*/
#pragma once

#include "AkVPLSrcCbxNode.h"
#include "AkVPLMixBusNode.h"
#include "AkLEngineCmds.h"

class __declspec(align(16)) AkVPL
{
public:
	AkListBare<CAkVPLSrcCbxNode>	m_listVPLSrcs;
	CAkVPLMixBusNode				m_MixBus;
	AkVPL *							m_pParent;
	AkUInt8							m_bFeedback : 1;
};
static_assert( sizeof( AkVPL ) == 0x120, "AkVPL: 2012 sizeof 0x120 (CreateVPLMixBus)" );

struct __declspec(align(16)) AkRunningVPL
{
	AkRunningVPL *		pNextItem;
	CAkVPLSrcCbxNode *	pCbx;
	AkVPL *				pBus;
	AkVPLState			state;
	void *				pFeedbackData;
	bool				bFeedbackVPL;
};

namespace AkAudioLibSettings
{
	extern AkUInt32 g_pipelineCoreFrequency;
	extern AkUInt32 g_pcWaitTime;
	extern AkUInt32 g_uLpfUpdatePeriod;
	void SetSampleFrequency( AkUInt32 in_uSampleFrequency );
}

class CAkLEngine
{
public:
	static AKRESULT Init();
	static void Term();
	static void Stop();
	static void ApplyGlobalSettings( AkPlatformInitSettings * io_pPDSettings );
	static void Perform();
	static AkUInt32 GetNumBufferNeededAndSubmit();
	static void StartVoice();
	static AKRESULT SetPanningRule( AkPanningRule in_panningRule );

	static AKRESULT AddSound( AkLECmd & io_cmd );
	static AKRESULT VPLTryConnectSource( CAkPBI * in_pContext, CAkVPLSrcCbxNode * in_pCbx );
	static CAkVPLSrcCbxNode * ResolveCommandVPL( AkLECmd & io_cmd );
	static void VPLDestroySource( CAkVPLSrcCbxNode * in_pCbx );
	static void StopMixBussesUsingThisSlot( const CAkUsageSlot * in_pSlot );

	static void SetBusVolume( AkUniqueID in_MixBusID, AkReal32 in_VolumeOffset );
	static void ForceBusVolume( AkUniqueID in_MixBusID, AkReal32 in_Volume );
	static void SetMasterBusVolume( AkReal32 in_VolumeOffset );
	static void ForceMasterBusVolume( AkReal32 in_Volume );
	static void BypassBusFx( AkUniqueID in_MixBusID, AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	static void BypassMasterBusFx( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask );
	static void BypassEnvFx( AkUniqueID in_EnvID, bool in_bIsBypassed );
	static void UpdateMasterBusFX( AkUInt32 in_uFXIndex );
	static void UpdateMixBusFX( AkUniqueID in_MixBusID, AkUInt32 in_uFXIndex );
	static void ForAllPluginParam( CAkFxBase * in_pFx, void ( *in_funcForAll )( AK::IAkPluginParam *, CAkRegisteredObj *, void * ), void * in_pCookie );

	static void * GetCachedAudioBuffer( AkUInt32 in_uSize );
	static void ReleaseCachedAudioBuffer( AkUInt32 in_uSize, void * in_pvBuffer );

	/** DISHONORED(diag, -akwav): every buffer the sink is given, 16-bit, appended to a WAV file. */
	static void SetWavCapture( const wchar_t * in_pszPath );

	static AkPanningRule					m_panningRule;
	static AkPipelineBufferBase				m_MasterOut;
	static CAkVPLFinalMixNode *				m_pFinalMixNode;
	static AkArray<AkVPL *, AkVPL *, ArrayPoolLEngineDefault, 8>	m_arrayVPLs;
	/** The environment (aux send) mix bus nodes, one per environment share set in use (agent IH4). */
	static CAkKeyList<AkUniqueID, CAkVPLMixBusNode *, 1, ArrayPoolLEngineDefault>	m_EnvBusList;
	static AkUInt32							m_uLastStarvationTime;
	static bool								m_bResetAudioDevice;
	static bool								m_bCoInitializeSucceeded;

private:
	struct CachedBuffers
	{
		AkUInt32	m_cItems;
		void *		m_Items[ 2 ];
	};

	static AKRESULT CreateLEnginePools();
	static void DestroyLEnginePools();
	static void HandleStarvation();
	static void GetBuffer();
	static void RunVPL( AkRunningVPL & io_runningVPL );
	static void FinishRun( CAkVPLSrcCbxNode * in_pCbx, AkVPLState & io_state );
	static CAkVPLSrcCbxNode * FindExistingVPLSrc( CAkPBI * in_pCtx );
	static AkVPL * GetVPLMixBus( CAkPBI * in_pCtx );
	static AkVPL * GetVPLMixBus( AK::CAkBusCtx & in_ctxBus, bool in_bForFeedback );
	static AkVPL * CreateVPLMixBus( AK::CAkBusCtx in_BusCtx, AkChannelMask in_uChannelMask, AkUInt16 in_usMaxFrames );
	static void DestroyVPLMixBus( AkVPL * in_pMixBus );
	static void DestroyAllVPLMixBusses();
	static CAkVPLMixBusNode * GetEnvironmentalBus( AkUniqueID in_EnvID, CAkPBI * in_pCtx );
	static CAkVPLMixBusNode * CreateEnvMixBus( AkUniqueID in_EnvID, AK::CAkBusCtx in_BusContext );
	static void DestroyEnvMixBus( CAkVPLMixBusNode * in_pMixBus );
	static void RemoveMixBusses();
	static void WriteWav( const AkPipelineBufferBase * in_pBuffer );

	static CachedBuffers	m_CachedAudioBuffers[ 24 ];
};
