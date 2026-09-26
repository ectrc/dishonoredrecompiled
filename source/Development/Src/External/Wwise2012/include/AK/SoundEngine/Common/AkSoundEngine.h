/*=============================================================================
	AkSoundEngine.h - Wwise 2012.1 sound-engine API.

	DISHONORED(written): every declaration below reproduces a demangled 2012 PDB signature from
	resources/docs/symbols/functions.csv; the 2012 rva is in the trailing comment of each one and the
	overload sets are the exe's own (four LoadBank id/string/memory forms, six SeekOnEvent forms, ...).
	The surface is the one Arkane actually calls: the 62 distinct AK:: entry points that appear in the
	decompiles of AkAudio, Engine's akbank/akevent and DishonoredGame's UDishonoredAudioSystem
	(build/agentAN/dec2013), plus their same-family neighbours where the signature was free.

	DISHONORED(layout): AkInitSettings (44 bytes) and AkPlatformInitSettings (64 bytes) are the 2012
	PDB's records field for field; hWnd is declared as struct HWND__ * exactly as the PDB has it, which
	keeps windows.h out of this header.
=============================================================================*/
#ifndef _AK_SOUND_ENGINE_H_
#define _AK_SOUND_ENGINE_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include <AK/SoundEngine/Common/AkModule.h>

struct HWND__;

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** DISHONORED(layout): 44 bytes. uPrepareEventMemoryPoolID is AkMemPoolId (`int` in the PDB record). */
struct AkInitSettings
{
	void ( *pfnAssertHook )( const char * in_pszExpression, const char * in_pszFileName, int in_lineNumber );
	AkUInt32	uMaxNumPaths;
	AkUInt32	uMaxNumTransitions;
	AkUInt32	uDefaultPoolSize;
	AkReal32	fDefaultPoolRatioThreshold;
	AkUInt32	uCommandQueueSize;
	AkMemPoolId	uPrepareEventMemoryPoolID;
	bool		bEnableGameSyncPreparation;
	AkUInt32	uContinuousPlaybackLookAhead;
	AkUInt32	uMonitorPoolSize;
	AkUInt32	uMonitorQueuePoolSize;
};

/** DISHONORED(layout): 64 bytes; uNumRefillsInVoice is AkUInt16 @36, eAudioQuality @40, threadMonitor @52. */
struct AkPlatformInitSettings
{
	struct HWND__ *		hWnd;
	AkThreadProperties	threadLEngine;
	AkThreadProperties	threadBankManager;
	AkUInt32			uLEngineDefaultPoolSize;
	AkReal32			fLEngineDefaultPoolRatioThreshold;
	AkUInt16			uNumRefillsInVoice;
	AkSoundQuality		eAudioQuality;
	AkChannelMask		uChannelMask;
	bool				bGlobalFocus;
	AkThreadProperties	threadMonitor;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

class IAkSoftwareCodec;

namespace AK
{
	class IAkPlugin;
	class IAkPluginParam;
	class IAkPluginMemAlloc;

	typedef IAkPlugin * ( *AkCreatePluginCallback )( IAkPluginMemAlloc * in_pAllocator );
	typedef IAkPluginParam * ( *AkCreateParamCallback )( IAkPluginMemAlloc * in_pAllocator );
	typedef IAkSoftwareCodec * ( *AkCreateFileSourceCallback )( void * in_pCtx );
	typedef IAkSoftwareCodec * ( *AkCreateBankSourceCallback )( void * in_pCtx );

	namespace SoundEngine
	{
		/** PrepareBank / PrepareEvent / PrepareGameSyncs direction. */
		enum PreparationType
		{
			Preparation_Load	= 0,
			Preparation_Unload	= 1
		};

		/** PrepareBank: metadata only, or metadata plus media. */
		enum AkBankContent
		{
			AkBankContent_StructureOnly	= 0,
			AkBankContent_All			= 1
		};

		enum AkActionOnEventType
		{
			AkActionOnEventType_Stop	= 0,
			AkActionOnEventType_Pause	= 1,
			AkActionOnEventType_Resume	= 2,
			AkActionOnEventType_Break	= 3
		};

		enum AkCommandPriority
		{
			AkCommandPriority_Game			= 0,
			AkCommandPriority_WwiseApp		= 1,
			AkCommandPriority_InitDefault	= 2,
			AkCommandPriority_None			= 3
		};

		enum MultiPositionType
		{
			MultiPositionType_SingleSource		= 0,
			MultiPositionType_MultiSources		= 1,
			MultiPositionType_MultiDirections	= 2
		};

		// -- lifetime ------------------------------------------------------------------------------
		bool IsInitialized();																	///< 2012 rva 0x916df0
		void GetDefaultInitSettings( AkInitSettings & out_settings );							///< 2012 rva 0x916e00
		void GetDefaultPlatformInitSettings( AkPlatformInitSettings & out_settings );			///< 2012 rva 0x916e50
		AKRESULT Init( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings );	///< 2012 rva 0x91ad50
		void Term();																			///< 2012 rva 0x91a090
		AKRESULT RenderAudio();																	///< 2012 rva 0x916e80

		// -- plug-ins ------------------------------------------------------------------------------
		AKRESULT RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback in_pCreateFunc, AkCreateParamCallback in_pCreateParamFunc );	///< 2012 rva 0x916e90
		AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc );	///< 2012 rva 0x916ea0

		// -- game objects --------------------------------------------------------------------------
		AKRESULT RegisterGameObj( AkGameObjectID in_gameObjectID, const char * in_pszObjName = "" );	///< 2012 rva 0x9179a0
		AKRESULT UnregisterGameObj( AkGameObjectID in_gameObjectID );							///< 2012 rva 0x9179f0
		AKRESULT UnregisterAllGameObj();														///< 2012 rva 0x917a30

		// -- positioning ---------------------------------------------------------------------------
		AKRESULT SetPosition( AkGameObjectID in_gameObjectID, const AkSoundPosition & in_Position, AkUInt32 in_uListenerIndex = 0xFFFFFFFF );	///< 2012 rva 0x918690
		AKRESULT SetMultiplePositions( AkGameObjectID in_gameObjectID, const AkSoundPosition * in_pPositions, AkUInt16 in_NumPositions, MultiPositionType in_eMultiPositionType = MultiPositionType_MultiDirections );	///< 2012 rva 0x916f20
		AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex = 0 );	///< 2012 rva 0x9170e0
		AKRESULT SetActiveListeners( AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask );	///< 2012 rva 0x9170a0
		AKRESULT SetAttenuationScalingFactor( AkGameObjectID in_gameObjectID, AkReal32 in_fAttenuationScalingFactor );	///< 2012 rva 0x917000
		AKRESULT SetListenerScalingFactor( AkUInt32 in_uListenerIndex, AkReal32 in_fAttenuationScalingFactor );	///< 2012 rva 0x917050
		AKRESULT SetListenerSpatialization( AkUInt32 in_uIndex, bool in_bSpatialized, AkSpeakerVolumes * in_pVolumeOffsets = NULL );	///< 2012 rva 0x917130
		AKRESULT SetListenerPipeline( AkUInt32 in_uIndex, bool in_bAudio, bool in_bFeedback );	///< 2012 rva 0x9171b0

		// -- game syncs ----------------------------------------------------------------------------
		AKRESULT SetRTPCValue( AkRtpcID in_rtpcID, AkRtpcValue in_value, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x917200
		AKRESULT SetRTPCValue( const AkOSChar * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x9186b0
		AKRESULT SetRTPCValue( const char * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x9186f0
		AKRESULT ResetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x917380
		AKRESULT ResetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x918920
		AKRESULT ResetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uValueChangeDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x918940

		AKRESULT SetSwitch( AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID );	///< 2012 rva 0x917290
		AKRESULT SetSwitch( const AkOSChar * in_pszSwitchGroup, const AkOSChar * in_pszSwitchState, AkGameObjectID in_gameObjectID );	///< 2012 rva 0x918730
		AKRESULT SetSwitch( const char * in_pszSwitchGroup, const char * in_pszSwitchState, AkGameObjectID in_gameObjectID );	///< 2012 rva 0x918770

		AKRESULT SetState( AkStateGroupID in_stateGroup, AkStateID in_state );					///< 2012 rva 0x918850
		AKRESULT SetState( const AkOSChar * in_pszStateGroup, const AkOSChar * in_pszState );	///< 2012 rva 0x9188a0
		AKRESULT SetState( const char * in_pszStateGroup, const char * in_pszState );			///< 2012 rva 0x9188e0

		AKRESULT PostTrigger( AkTriggerID in_triggerID, AkGameObjectID in_gameObjectID );		///< 2012 rva 0x9172e0
		AKRESULT PostTrigger( const AkOSChar * in_pszTrigger, AkGameObjectID in_gameObjectID );	///< 2012 rva 0x9187b0
		AKRESULT PostTrigger( const char * in_pszTrigger, AkGameObjectID in_gameObjectID );		///< 2012 rva 0x918800

		// -- events --------------------------------------------------------------------------------
		AkPlayingID PostEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags = 0, AkCallbackFunc in_pfnCallback = NULL, void * in_pCookie = NULL, AkUInt32 in_cExternals = 0, AkExternalSourceInfo * in_pExternalSources = NULL, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91ae90
		AkPlayingID PostEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags = 0, AkCallbackFunc in_pfnCallback = NULL, void * in_pCookie = NULL, AkUInt32 in_cExternals = 0, AkExternalSourceInfo * in_pExternalSources = NULL, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91a4f0
		AkPlayingID PostEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags = 0, AkCallbackFunc in_pfnCallback = NULL, void * in_pCookie = NULL, AkUInt32 in_cExternals = 0, AkExternalSourceInfo * in_pExternalSources = NULL, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91a580

		AKRESULT ExecuteActionOnEvent( AkUniqueID in_eventID, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91a610
		AKRESULT ExecuteActionOnEvent( const AkOSChar * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91a690
		AKRESULT ExecuteActionOnEvent( const char * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear, AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID );	///< 2012 rva 0x91a6b0

		void StopAll( AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT );					///< 2012 rva 0x918350
		void StopPlayingID( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );	///< 2012 rva 0x918390

		AKRESULT SeekOnEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a6d0
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a740
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a760
		AKRESULT SeekOnEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a780
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a800
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker = false );	///< 2012 rva 0x91a830

		void CancelEventCallbackCookie( void * in_pCookie );										///< 2012 rva 0x917880
		void CancelEventCallback( AkPlayingID in_playingID );									///< 2012 rva 0x9178a0
		void CancelBankCallbackCookie( void * in_pCookie );										///< 2012 rva 0x917fc0
		AKRESULT GetSourcePlayPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPosition, bool in_bExtrapolate = true );	///< 2012 rva 0x9178c0

		// -- name hashing --------------------------------------------------------------------------
		AkUInt32 GetIDFromString( const AkOSChar * in_pszString );								///< 2012 rva 0x918570
		AkUInt32 GetIDFromString( const char * in_pszString );									///< 2012 rva 0x918600
		AkBankID GetBankIDFromString( const AkOSChar * in_pszString );							///< 2012 rva 0x918a70

		// -- banks ---------------------------------------------------------------------------------
		AKRESULT LoadBank( const AkOSChar * in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID );	///< 2012 rva 0x918b10
		AKRESULT LoadBank( const char * in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID );	///< 2012 rva 0x918bd0
		AKRESULT LoadBank( AkBankID in_bankID, AkMemPoolId in_memPoolId );						///< 2012 rva 0x917cf0
		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankID & out_bankID );	///< 2012 rva 0x917d80
		AKRESULT LoadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID );	///< 2012 rva 0x918c60
		AKRESULT LoadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID );	///< 2012 rva 0x918d00
		AKRESULT LoadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId );	///< 2012 rva 0x917e10
		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankID & out_bankID );	///< 2012 rva 0x917e70

		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkMemPoolId * out_pMemPoolId = NULL );	///< 2012 rva 0x918d70
		AKRESULT UnloadBank( const char * in_pszString, AkMemPoolId * out_pMemPoolId = NULL );	///< 2012 rva 0x918dc0
		AKRESULT UnloadBank( AkBankID in_bankID, AkMemPoolId * out_pMemPoolId = NULL );			///< 2012 rva 0x917ec0
		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x918de0
		AKRESULT UnloadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x918e70
		AKRESULT UnloadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x917f60

		AKRESULT ClearBanks();																	///< 2012 rva 0x9189e0
		AKRESULT SetBankLoadIOSettings( AkReal32 in_fThroughput, AkPriority in_priority );		///< 2012 rva 0x917a90
		void DefaultBankCallbackFunc( AkUInt32 in_bankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie );	///< 2012 rva 0x917a70

		// -- prepare (metadata-only bank loading, the seek-free BK_ path) ---------------------------
		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x919a50
		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x919aa0
		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x918ee0
		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x918f40
		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x918f90
		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags = AkBankContent_All );	///< 2012 rva 0x917fe0

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent );	///< 2012 rva 0x918fb0
		AKRESULT PrepareEvent( PreparationType in_PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent );	///< 2012 rva 0x9190b0
		AKRESULT PrepareEvent( PreparationType in_PreparationType, AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent );	///< 2012 rva 0x918040
		AKRESULT PrepareEvent( PreparationType in_PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x9191b0
		AKRESULT PrepareEvent( PreparationType in_PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x9192b0
		AKRESULT PrepareEvent( PreparationType in_PreparationType, AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie );	///< 2012 rva 0x9180a0

		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, const AkOSChar * in_pszGroupName, const AkOSChar ** in_ppszGameSyncName, AkUInt32 in_uNumGameSyncs );	///< 2012 rva 0x919630
		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, const char * in_pszGroupName, const char ** in_ppszGameSyncName, AkUInt32 in_uNumGameSyncs );	///< 2012 rva 0x919760
		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, AkUInt32 in_GroupID, AkUInt32 * in_paGameSyncID, AkUInt32 in_uNumGameSyncs );	///< 2012 rva 0x918190
		AKRESULT ClearPreparedEvents();															///< 2012 rva 0x9180d0

		// -- environments / obstruction ------------------------------------------------------------
		AKRESULT SetEffect( AkUniqueID in_audioNodeID, AkUInt32 in_uFXIndex, AkUniqueID in_shareSetID );	///< 2012 rva 0x918200
		AKRESULT SetEnvironmentVolume( AkAuxBusID in_envID, AkReal32 in_fVolume );				///< 2012 rva 0x918960
		AKRESULT SetEnvironmentVolumes( AkAuxBusID in_envID, const AkSpeakerVolumes & in_Volumes );	///< 2012 rva 0x9174f0
		AKRESULT BypassEnvironment( AkAuxBusID in_envID, bool in_bIsBypassed );					///< 2012 rva 0x917550
		AKRESULT SetGameObjectEnvironmentsValues( AkGameObjectID in_gameObjectID, AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues );	///< 2012 rva 0x917400
		AKRESULT SetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 in_fControlValue );	///< 2012 rva 0x9174b0
		AKRESULT SetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32 in_uListener, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel );	///< 2012 rva 0x917590

		// -- global settings -----------------------------------------------------------------------
		AkChannelMask GetSpeakerConfiguration();												///< 2012 rva 0x916e70
		AkPanningRule GetPanningRule();															///< 2012 rva 0x916e60
		AKRESULT SetPanningRule( AkPanningRule in_ePanningRule );								///< 2012 rva 0x917720
		AKRESULT SetMaxNumVoicesLimit( AkUInt16 in_maxNumberVoices );							///< 2012 rva 0x9189b0
		AKRESULT SetVolumeThreshold( AkReal32 in_fVolumeThresholdDB );							///< 2012 rva 0x918990

		// -- global callbacks ----------------------------------------------------------------------
		AKRESULT RegisterGlobalCallback( AkGlobalCallbackFunc in_pCallback );					///< 2012 rva 0x919c60
		AKRESULT UnregisterGlobalCallback( AkGlobalCallbackFunc in_pCallback );					///< 2012 rva 0x919c90
		AKRESULT AddBehavioralExtension( AkGlobalCallbackFunc in_pCallback );					///< 2012 rva 0x919be0
		AKRESULT RemoveBehavioralExtension( AkGlobalCallbackFunc in_pCallback );					///< 2012 rva 0x919c50
		void AddExternalStateHandler( bool ( *in_pHandler )( AkStateGroupID, AkStateID ) );		///< 2012 rva 0x918250
	}
}

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkInitSettings) == 44, "AkInitSettings: 2012 PDB sizeof 44");
static_assert(sizeof(AkPlatformInitSettings) == 64, "AkPlatformInitSettings: 2012 PDB sizeof 64");
#endif

#endif // _AK_SOUND_ENGINE_H_
