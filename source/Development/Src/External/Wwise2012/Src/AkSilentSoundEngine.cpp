/*=============================================================================
	AkSilentSoundEngine.cpp - AK::SoundEngine's playback API (wwise\sdk\source\soundengine\akaudiolib\win32\
	akaudiolib.cpp): every call writes a message into CAkAudioMgr's queue, which the audio thread ("AK::EventManager")
	drains into the renderer and the lower engine; PostEvent registers its playing id with CAkPlayingMgr first.

	DISHONORED(port): agent AN wrote this file as the whole silent engine, agent IH1 (wave 26) moved the bank side to the
	ported runtime, agent IH2 (wave 26) replaced the silent playback with the ported one. 2012 rvas:
	  RenderAudio 0x916e80, RegisterPlugin 0x916e90, RegisterCodec 0x916ea0, SetPositionInternal 0x916eb0 (SetPosition
	  0x918690), SetMultiplePositions 0x916f20, SetAttenuationScalingFactor 0x917000, SetListenerScalingFactor 0x917050,
	  SetActiveListeners 0x9170a0, SetListenerPosition 0x9170e0, SetListenerSpatialization 0x917130,
	  SetListenerPipeline 0x9171b0, SetGameObjectEnvironmentsValues 0x917400, SetGameObjectDryLevelValue 0x9174b0,
	  SetObjectObstructionAndOcclusion 0x917590, SetPanningRule 0x917720, CancelEventCallbackCookie 0x917880,
	  CancelEventCallback 0x9178a0, GetSourcePlayPosition 0x9178c0, RegisterGameObj 0x9179a0, UnregisterGameObj
	  0x9179f0, UnregisterAllGameObj 0x917a30, StopAll 0x918350, StopPlayingID 0x918390, AddBehavioralExtension 0x919be0,
	  RemoveBehavioralExtension 0x919c50, RegisterGlobalCallback 0x919c60, UnregisterGlobalCallback 0x919c90, PostEvent
	  0x91a400 / 0x91a4f0 / 0x91a580 / 0x91ae90, ExecuteActionOnEvent 0x91a610 / 0x91a690 / 0x91a6b0, SeekOnEvent
	  0x91a6d0..0x91a830, Query::GetEventIDFromPlayingID 0x91b1b0, GetGameObjectFromPlayingID 0x91b1d0,
	  GetPlayingIDsFromGameObject 0x91b1f0, GetListenerPosition 0x91af20, GetListenerSpatialization 0x91af50.
	Deviations (named):
	  * Agent IH3 fork R: SetRTPCValue 0x917200 / 0x9186b0 / 0x9186f0, ResetRTPCValue 0x917380 / 0x918920 / 0x918940,
	    SetSwitch 0x917290 / 0x918730 / 0x918770, SetState 0x918850 / 0x9188a0 / 0x9188e0, PostTrigger 0x9172e0 /
	    0x9187b0 / 0x918800, SetEnvironmentVolumes 0x9174f0, SetEnvironmentVolume 0x918960, BypassEnvironment 0x917550,
	    AddExternalStateHandler 0x918250 and Query::GetRTPCValue 0x91b360.., GetSwitch 0x91b480.., GetEnvironmentVolumes
	    0x91b050, GetEnvironmentBypass 0x91b0b0, GetEnvironmentVolume 0x91b220 are retail's (queued to the RTPC / state
	    / environment managers). Names are hashed through AkSilent::RegisterName / NameHash (GetIDFromString's FNV hash,
	    remembered for the log). Query::GetActiveGameObjects (the AkArray form) is not declared by our public header and
	    has no caller; CAkRegistryMgr::GetActiveGameObjects is ported behind it.
	  * DishonoredWwise::SetDurationHint (agent ID) is a no-op shim now that a voice ends at its media end.
	  * SetEffect is IH4's and answers AK_Success without queueing.
	  * Query::GetMaxRadius is IH3's (AkPositioning.cpp, 2012 0x91b180).
	  * PostEvent logs every post (and -akdiag every unknown event) through the backend log hook: diagnostic.
=============================================================================*/
#include <AkSink.h>
#include <AkBankMgr.h>
#include <AkRegistryMgr.h>
#include <AkAudioMgr.h>
#include <AkPlayingMgr.h>
#include <AkListener.h>
#include <AkLEngine.h>
#include <AkVPLNode.h>
#include <AkPositionRepository.h>
#include <AkDynamicSequence.h>
#include "AkSilentInternal.h"

AKRESULT AkAudioLibInit( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings );
void AkAudioLibTerm();

extern AkArray<AkGlobalCallbackFunc, AkGlobalCallbackFunc, ArrayPoolDefault, 4> g_aBehavioralExtensions;

static volatile LONG g_PlayingID = 0;

/** DISHONORED(deviation, agent IH2): a call after AK::SoundEngine::Term answers AK_Fail instead of writing into the
    deleted audio manager (retail dereferences NULL). The glue reaches StopAll from garbage collection after
    UAkAudioDevice::Teardown on exit (UAkComponent::FinishDestroy -> Stop), which the silent backend used to swallow. */
static AKRESULT Enqueue( AkQueuedMsg & in_rItem, AkUInt32 in_uSize )
{
	return g_pAudioMgr ? g_pAudioMgr->Enqueue( in_rItem, in_uSize ) : AK_Fail;
}

namespace DishonoredWwise
{
	bool SetDurationHint( AkPlayingID in_playingID, AkReal32 in_fSeconds )
	{
		return in_playingID != AK_INVALID_PLAYING_ID;
	}

	void SetWavCapture( const AkOSChar * in_pszPath )
	{
		CAkLEngine::SetWavCapture( in_pszPath );
	}
}

namespace AK
{
	namespace SoundEngine
	{
		// -- lifetime ------------------------------------------------------------------------------

		bool IsInitialized()
		{
			return AkSilent::Get().bSoundEngineInit;
		}

		AKRESULT Init( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings )
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !AK::MemoryMgr::IsInitialized() )
			{
				return AK_MemManagerNotInitialized;
			}
			if( !IAkStreamMgr::Get() )
			{
				return AK_StreamMgrNotInitialized;
			}
			const AKRESULT eResult = AkAudioLibInit( in_pSettings, in_pPlatformSettings );
			if( eResult != AK_Success )
			{
				AkAudioLibTerm();
				return eResult;
			}
			e.bSoundEngineInit = true;
			AkSilent::Logf( L"SoundEngine::Init: %s sink, speaker mask 0x%x, command queue %u",
				g_pAkSink && g_pAkSink->m_IsHardwareDevicePresent ? L"DirectSound" : L"dummy", g_pAkSink ? g_pAkSink->m_SpeakersConfig : 0,
				in_pSettings ? in_pSettings->uCommandQueueSize : 0 );
			return AK_Success;
		}

		void Term()
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return;
			}
			const DishonoredWwise::Stats & stats = DishonoredWwise::GetStats();
			AkSilent::Logf( L"SoundEngine::Term: %u banks loaded, %u prepared, %u missing, %u events posted, %u RTPC / %u switch / %u state sets",
				stats.BanksLoaded, stats.BanksPrepared, stats.BanksMissing, stats.EventsPosted,
				stats.RtpcSets, stats.SwitchSets, stats.StateSets );
			e.Banks.clear();
			e.bSoundEngineInit = false;
			AkAudioLibTerm();
		}

		AKRESULT RenderAudio()
		{
			return g_pAudioMgr ? g_pAudioMgr->RenderAudio() : AK_Fail;
		}

		// -- plug-ins ------------------------------------------------------------------------------

		/** 2012 0x916e90 -> CAkEffectsMgr::RegisterPlugin; an effect needs its plug-in interfaces (IH4) to run. */
		AKRESULT RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback in_pCreateFunc, AkCreateParamCallback in_pCreateParamFunc )
		{
			return CAkEffectsMgr::RegisterPlugin( in_eType, in_ulCompanyID, in_ulPluginID, in_pCreateFunc, in_pCreateParamFunc );
		}

		AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc )
		{
			return CAkEffectsMgr::RegisterCodec( in_ulCompanyID, in_ulCodecID, in_pFileCreateFunc, in_pBankCreateFunc );
		}

		// -- game objects --------------------------------------------------------------------------

		AKRESULT RegisterGameObj( AkGameObjectID in_gameObjectID, const char * in_pszObjName )
		{
			if( !in_gameObjectID || in_gameObjectID == AK_INVALID_GAME_OBJECT )
			{
				return AK_Fail;
			}
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_RegisterGameObj;
			Item.reggameobj.GameObjID = in_gameObjectID;
			Item.reggameobj.pMonitorData = NULL;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_RegisterGameObj>() );
		}

		AKRESULT UnregisterGameObj( AkGameObjectID in_gameObjectID )
		{
			if( !in_gameObjectID )
			{
				return AK_Fail;
			}
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_UnregisterGameObj;
			Item.unreggameobj.GameObjID = in_gameObjectID;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_UnregisterGameObj>() );
		}

		AKRESULT UnregisterAllGameObj()
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_UnregisterGameObj;
			Item.unreggameobj.GameObjID = AK_INVALID_GAME_OBJECT;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_UnregisterGameObj>() );
		}

		// -- positioning ---------------------------------------------------------------------------

		AKRESULT SetPosition( AkGameObjectID in_gameObjectID, const AkSoundPosition & in_Position, AkUInt32 in_ulListenerIndex )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjPosition;
			Item.gameobjpos.GameObjID = in_gameObjectID;
			Item.gameobjpos.Position = in_Position;
			Item.gameobjpos.uListenerIndex = in_ulListenerIndex;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjPosition>() );
		}

		AKRESULT SetMultiplePositions( AkGameObjectID in_gameObjectID, const AkSoundPosition * in_pPositions, AkUInt16 in_NumPositions, MultiPositionType in_eMultiPositionType )
		{
			if( in_eMultiPositionType > MultiPositionType_MultiDirections || ( in_NumPositions && !in_pPositions ) )
			{
				return AK_InvalidParameter;
			}
			const AkUInt32 uAllocSize = AkQueuedMsg::Sizeof_GameObjMultiPosition( in_NumPositions );
			if( uAllocSize > AkSparseChunkRing::GetChunkSize() )
			{
				return AK_Fail;
			}
			AkQueuedMsg * pItem = (AkQueuedMsg *)_alloca( uAllocSize < sizeof( AkQueuedMsg ) ? sizeof( AkQueuedMsg ) : uAllocSize );
			pItem->type = QueuedMsgType_GameObjMultiPosition;
			pItem->gameObjMultiPos.GameObjID = in_gameObjectID;
			pItem->gameObjMultiPos.uNumPositions = in_NumPositions;
			pItem->gameObjMultiPos.eMultiPositionType = in_eMultiPositionType;
			for( AkUInt16 i = 0; i < in_NumPositions; ++i )
			{
				pItem->gameObjMultiPos.aMultiPosition[ i ] = in_pPositions[ i ];
			}
			return Enqueue( *pItem, uAllocSize );
		}

		AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_ListenerPosition;
			Item.listpos.uListenerIndex = in_uIndex;
			Item.listpos.Position = in_Position;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ListenerPosition>() );
		}

		AKRESULT SetActiveListeners( AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjActiveListeners;
			Item.gameobjactlist.GameObjID = in_gameObjectID;
			Item.gameobjactlist.uListenerMask = in_uListenerMask;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjActiveListeners>() );
		}

		AKRESULT SetAttenuationScalingFactor( AkGameObjectID in_gameObjectID, AkReal32 in_fAttenuationScalingFactor )
		{
			if( in_fAttenuationScalingFactor <= 0.f )
			{
				return AK_InvalidParameter;
			}
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjScalingFactor;
			Item.gameobjscalingfactor.GameObjID = in_gameObjectID;
			Item.gameobjscalingfactor.fValue = in_fAttenuationScalingFactor;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjScalingFactor>() );
		}

		AKRESULT SetListenerScalingFactor( AkUInt32 in_uListenerIndex, AkReal32 in_fListenerScalingFactor )
		{
			if( in_fListenerScalingFactor <= 0.f )
			{
				return AK_InvalidParameter;
			}
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_ListenerScalingFactor;
			Item.listenerscalingfactor.uListenerIndex = in_uListenerIndex;
			Item.listenerscalingfactor.fValue = in_fListenerScalingFactor;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ListenerScalingFactor>() );
		}

		AKRESULT SetListenerSpatialization( AkUInt32 in_uIndex, bool in_bSpatialized, AkSpeakerVolumes * in_pVolumeOffsets )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_ListenerSpatialization;
			Item.listspat.uListenerIndex = in_uIndex;
			Item.listspat.bSpatialized = in_bSpatialized;
			Item.listspat.bSetVolumes = in_pVolumeOffsets != NULL;
			if( in_pVolumeOffsets )
			{
				Item.listspat.Volumes = *in_pVolumeOffsets;
			}
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ListenerSpatialization>() );
		}

		AKRESULT SetListenerPipeline( AkUInt32 in_uIndex, bool in_bAudio, bool in_bFeedback )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_ListenerPipeline;
			Item.listpipe.uListenerIndex = in_uIndex;
			Item.listpipe.bAudio = in_bAudio;
			Item.listpipe.bFeedback = in_bFeedback;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ListenerPipeline>() );
		}

		// -- game syncs (IH3 fork R: queued to CAkRTPCMgr / CAkStateMgr as retail) ----------------

		/** 2012 0x917200: with a fade the message carries its TransParams (type 3), else type 2. */
		AKRESULT SetRTPCValue( AkRtpcID in_rtpcID, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			++AkSilent::MutableStats().RtpcSets;
			AkQueuedMsg Item;
			Item.rtpc.ID = in_rtpcID;
			Item.rtpc.Value = in_value;
			Item.rtpc.GameObjID = in_gameObjectID;
			if( in_uValueChangeDuration )
			{
				Item.type = QueuedMsgType_RTPCWithTransition;
				Item.rtpcWithTransition.transParams.TransitionTime = in_uValueChangeDuration;
				Item.rtpcWithTransition.transParams.eFadeCurve = in_eFadeCurve;
				return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_RtpcWithTransition>() );
			}
			Item.type = QueuedMsgType_RTPC;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_Rtpc>() );
		}

		/** 2012 0x9186b0: an unknown name is AK_IDNotFound. */
		AKRESULT SetRTPCValue( const AkOSChar * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			const AkRtpcID id = AkSilent::RegisterName( in_pszRtpcName );
			return id ? SetRTPCValue( id, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve ) : AK_IDNotFound;
		}

		/** 2012 0x9186f0. */
		AKRESULT SetRTPCValue( const char * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			const AkRtpcID id = AkSilent::RegisterName( in_pszRtpcName );
			return id ? SetRTPCValue( id, in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve ) : AK_IDNotFound;
		}

		/** 2012 0x917380: with a fade type 6 carries its TransParams, else type 5. */
		AKRESULT ResetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			AkQueuedMsg Item;
			Item.resetrtpcvalue.ParamID = in_rtpcID;
			Item.resetrtpcvalue.GameObjID = in_gameObjectID;
			if( in_uValueChangeDuration )
			{
				Item.type = QueuedMsgType_ResetRTPCValueWithTransition;
				Item.resetrtpcvalueWithTransition.transParams.TransitionTime = in_uValueChangeDuration;
				Item.resetrtpcvalueWithTransition.transParams.eFadeCurve = in_eFadeCurve;
				return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ResetRtpcValueWithTransition>() );
			}
			Item.type = QueuedMsgType_ResetRTPCValue;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_ResetRTPCValue>() );
		}

		/** 2012 0x918920. */
		AKRESULT ResetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			const AkRtpcID id = AkSilent::RegisterName( in_pszRtpcName );
			return id ? ResetRTPCValue( id, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve ) : AK_IDNotFound;
		}

		/** 2012 0x918940. */
		AKRESULT ResetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			const AkRtpcID id = AkSilent::RegisterName( in_pszRtpcName );
			return id ? ResetRTPCValue( id, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve ) : AK_IDNotFound;
		}

		/** 2012 0x917290. */
		AKRESULT SetSwitch( AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID )
		{
			++AkSilent::MutableStats().SwitchSets;
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_Switch;
			Item.setswitch.GameObjID = in_gameObjectID;
			Item.setswitch.SwitchGroupID = in_switchGroup;
			Item.setswitch.SwitchStateID = in_switchState;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_Switch>() );
		}

		/** 2012 0x918730. */
		AKRESULT SetSwitch( const AkOSChar * in_pszSwitchGroup, const AkOSChar * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			const AkSwitchGroupID group = AkSilent::RegisterName( in_pszSwitchGroup );
			const AkSwitchStateID state = AkSilent::RegisterName( in_pszSwitchState );
			return group && state ? SetSwitch( group, state, in_gameObjectID ) : AK_IDNotFound;
		}

		/** 2012 0x918770. */
		AKRESULT SetSwitch( const char * in_pszSwitchGroup, const char * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			const AkSwitchGroupID group = AkSilent::RegisterName( in_pszSwitchGroup );
			const AkSwitchStateID state = AkSilent::RegisterName( in_pszSwitchState );
			return group && state ? SetSwitch( group, state, in_gameObjectID ) : AK_IDNotFound;
		}

		/** 2012 0x918850 / 0x917320: the state "none" (FNV of "none", 748895195) is state 0. */
		AKRESULT SetState( AkStateGroupID in_stateGroup, AkStateID in_state )
		{
			++AkSilent::MutableStats().StateSets;
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_State;
			Item.setstate.StateGroupID = in_stateGroup;
			Item.setstate.StateID = in_state == 748895195 ? 0 : in_state;
			Item.setstate.bSkipTransition = false;
			Item.setstate.bSkipExtension = false;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_State>() );
		}

		/** 2012 0x9188a0. */
		AKRESULT SetState( const AkOSChar * in_pszStateGroup, const AkOSChar * in_pszState )
		{
			const AkStateGroupID group = AkSilent::RegisterName( in_pszStateGroup );
			const AkStateID state = AkSilent::RegisterName( in_pszState );
			return group && state ? SetState( group, state ) : AK_IDNotFound;
		}

		/** 2012 0x9188e0. */
		AKRESULT SetState( const char * in_pszStateGroup, const char * in_pszState )
		{
			const AkStateGroupID group = AkSilent::RegisterName( in_pszStateGroup );
			const AkStateID state = AkSilent::RegisterName( in_pszState );
			return group && state ? SetState( group, state ) : AK_IDNotFound;
		}

		/** 2012 0x9172e0. */
		AKRESULT PostTrigger( AkTriggerID in_triggerID, AkGameObjectID in_gameObjectID )
		{
			++AkSilent::MutableStats().TriggersPosted;
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_Trigger;
			Item.trigger.GameObjID = in_gameObjectID;
			Item.trigger.TriggerID = in_triggerID;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_Trigger>() );
		}

		/** 2012 0x9187b0. */
		AKRESULT PostTrigger( const AkOSChar * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			const AkTriggerID id = AkSilent::RegisterName( in_pszTrigger );
			return id ? PostTrigger( id, in_gameObjectID ) : AK_IDNotFound;
		}

		/** 2012 0x918800. */
		AKRESULT PostTrigger( const char * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			const AkTriggerID id = AkSilent::RegisterName( in_pszTrigger );
			return id ? PostTrigger( id, in_gameObjectID ) : AK_IDNotFound;
		}

		// -- events --------------------------------------------------------------------------------

		/** 2012 0x91a400: an event absent from every loaded bank answers AK_INVALID_PLAYING_ID; otherwise a new playing id
		    is registered with its callback (one active action: the event message) and the event message is queued. */
		static AkPlayingID PostEvent( AkUniqueID in_ulEventID, AkGameObjectID in_GameObjID, AkUInt32 in_uiFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkCustomParamType * in_pCustomParam, AkPlayingID in_PlayingID )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_Event;
			if( in_pCustomParam )
			{
				Item.event.CustomParam = *in_pCustomParam;
			}
			else
			{
				memset( &Item.event.CustomParam, 0, sizeof( Item.event.CustomParam ) );
			}
			Item.event.Event = g_pIndex && g_pAudioMgr ? g_pIndex->m_idxEvents.GetPtrAndAddRef( in_ulEventID ) : NULL;
			const AkOSChar * pszName = DishonoredWwise::NameForID( in_ulEventID );
			if( !Item.event.Event )
			{
				++AkSilent::MutableStats().EventsUnknown;
				AKDIAG( L"akdiag: PostEvent 0x%08x (%s): not in any loaded bank -> AK_INVALID_PLAYING_ID", in_ulEventID, pszName ? pszName : L"<id only>" );
				return AK_INVALID_PLAYING_ID;
			}
			Item.event.TargetPlayingID = in_PlayingID;
			Item.event.GameObjID = in_GameObjID;
			Item.event.PlayingID = (AkPlayingID)InterlockedIncrement( &g_PlayingID );
			if( g_pPlayingMgr->AddPlayingID( Item.event, in_pfnCallback, in_pCookie, in_uiFlags, Item.event.Event->key ) != AK_Success )
			{
				Item.event.Event->Release();
				return AK_INVALID_PLAYING_ID;
			}
			Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_Event>() );
			++AkSilent::MutableStats().EventsPosted;
			AkSilent::Logf( L"PostEvent %s (id 0x%08x) on game object %u -> playing id %u", pszName ? pszName : L"<id only>",
				in_ulEventID, (AkUInt32)in_GameObjID, Item.event.PlayingID );
			return Item.event.PlayingID;
		}

		/** 2012 0x91ae90 (and the named forms 0x91a4f0 / 0x91a580): the external sources travel ref-counted in the custom
		    parameter and are released again when the post fails. */
		AkPlayingID PostEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			AkCustomParamType customParam;
			AkCustomParamType * pCustomParam = NULL;
			if( in_cExternals )
			{
				memset( &customParam, 0, sizeof( customParam ) );
				customParam.pExternalSrcs = AkExternalSourceArray::Create( in_cExternals, in_pExternalSources );
				if( !customParam.pExternalSrcs )
				{
					return AK_INVALID_PLAYING_ID;
				}
				pCustomParam = &customParam;
			}
			const AkPlayingID playingID = PostEvent( in_eventID, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, pCustomParam, in_PlayingID );
			if( !playingID && pCustomParam )
			{
				pCustomParam->pExternalSrcs->Release();
			}
			return playingID;
		}

		AkPlayingID PostEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		AkPlayingID PostEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		// -- dynamic sequences (agent IH3 fork C; Dishonored never calls them) ------------------------------------

		namespace DynamicSequence
		{
			/** 2012 0x9178f0: a new playing id owns the sequence; the audio thread binds it to the game object. */
			AkPlayingID Open( AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, DynamicSequenceType in_eDynamicSequenceType )
			{
				AkQueuedMsg Item;
				Item.type = QueuedMsgType_OpenDynamicSequence;
				Item.opendynamicsequence.PlayingID = (AkPlayingID)InterlockedIncrement( &g_PlayingID );
				Item.opendynamicsequence.pDynamicSequence = CAkDynamicSequence::Create( Item.opendynamicsequence.PlayingID, in_eDynamicSequenceType );
				if( !Item.opendynamicsequence.pDynamicSequence )
				{
					return AK_INVALID_PLAYING_ID;
				}
				Item.opendynamicsequence.GameObjID = in_gameObjectID;
				Item.opendynamicsequence.TargetPlayingID = AK_INVALID_PLAYING_ID;
				memset( &Item.opendynamicsequence.CustomParam, 0, sizeof( Item.opendynamicsequence.CustomParam ) );
				if( g_pPlayingMgr->AddPlayingID( Item.opendynamicsequence, in_pfnCallback, in_pCookie, in_uFlags, AK_INVALID_UNIQUE_ID ) != AK_Success )
				{
					Item.opendynamicsequence.pDynamicSequence->Release();
					return AK_INVALID_PLAYING_ID;
				}
				Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_OpenDynamicSequence>() );
				return Item.opendynamicsequence.PlayingID;
			}

			/** The shared body of Play 0x91a9d0, Pause 0x91aa50, Resume 0x91aad0, Stop 0x91ab50, Break 0x91abd0, Close 0x91ac50:
			    a closed sequence refuses; the message keeps the reference GetPtrAndAddRef took. */
			static AKRESULT EnqueueCommand( AkPlayingID in_playingID, AkQueuedMsg_DynamicSequenceCmd::Command in_eCommand, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
			{
				CAkDynamicSequence * pSequence = g_pIndex->m_idxDynamicSequences.GetPtrAndAddRef( in_playingID );
				if( !pSequence )
				{
					return AK_Fail;
				}
				if( pSequence->m_bClosed )
				{
					pSequence->Release();
					return AK_Fail;
				}
				if( in_eCommand == AkQueuedMsg_DynamicSequenceCmd::Close )
				{
					pSequence->m_bClosed = true;
				}
				AkQueuedMsg Item;
				Item.type = QueuedMsgType_DynamicSequenceCmd;
				Item.dynamicsequencecmd.pDynamicSequence = pSequence;
				Item.dynamicsequencecmd.eCommand = in_eCommand;
				Item.dynamicsequencecmd.uTransitionDuration = in_uTransitionDuration;
				Item.dynamicsequencecmd.eFadeCurve = in_eFadeCurve;
				return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_DynamicSequenceCmd>() );
			}

			AKRESULT Play( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Play, in_uTransitionDuration, in_eFadeCurve );
			}

			AKRESULT Pause( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Pause, in_uTransitionDuration, in_eFadeCurve );
			}

			AKRESULT Resume( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Resume, in_uTransitionDuration, in_eFadeCurve );
			}

			AKRESULT Stop( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Stop, in_uTransitionDuration, in_eFadeCurve );
			}

			AKRESULT Break( AkPlayingID in_playingID )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Break, 0, AkCurveInterpolation_Linear );
			}

			AKRESULT Close( AkPlayingID in_playingID )
			{
				return EnqueueCommand( in_playingID, AkQueuedMsg_DynamicSequenceCmd::Close, 0, AkCurveInterpolation_Linear );
			}

			/** 2012 0x91acd0: the playlist stays locked until UnlockPlaylist. */
			Playlist * LockPlaylist( AkPlayingID in_playingID )
			{
				CAkDynamicSequence * pSequence = g_pIndex->m_idxDynamicSequences.GetPtrAndAddRef( in_playingID );
				if( !pSequence )
				{
					return NULL;
				}
				pSequence->m_lockPlaylist.Lock();
				pSequence->Release();
				return &pSequence->m_playList;
			}

			/** 2012 0x91ad10. */
			AKRESULT UnlockPlaylist( AkPlayingID in_playingID )
			{
				CAkDynamicSequence * pSequence = g_pIndex->m_idxDynamicSequences.GetPtrAndAddRef( in_playingID );
				if( !pSequence )
				{
					return AK_Fail;
				}
				pSequence->UnlockPlaylist();
				pSequence->Release();
				return AK_Success;
			}
		}

		AKRESULT ExecuteActionOnEvent( AkUniqueID in_eventID, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_EventAction;
			Item.eventAction.Event = g_pIndex ? g_pIndex->m_idxEvents.GetPtrAndAddRef( in_eventID ) : NULL;
			if( !Item.eventAction.Event )
			{
				return AK_Fail;
			}
			Item.eventAction.GameObjID = in_gameObjectID;
			Item.eventAction.eActionToExecute = in_ActionType;
			Item.eventAction.uTransitionDuration = in_uTransitionDuration;
			Item.eventAction.eFadeCurve = in_eFadeCurve;
			Item.eventAction.TargetPlayingID = in_PlayingID;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_EventAction>() );
		}

		AKRESULT ExecuteActionOnEvent( const AkOSChar * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		AKRESULT ExecuteActionOnEvent( const char * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		/** 2012 0x91a6d0 (time) / 0x91a780 (percent). */
		static AKRESULT SeekOnEventInternal( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, AkReal32 in_fPercent, bool in_bRelative, bool in_bSeekToNearestMarker )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_Seek;
			Item.seek.Event = g_pIndex ? g_pIndex->m_idxEvents.GetPtrAndAddRef( in_eventID ) : NULL;
			if( !Item.seek.Event )
			{
				return AK_Fail;
			}
			Item.seek.GameObjID = in_gameObjectID;
			if( in_bRelative )
			{
				Item.seek.fPercent = in_fPercent;
			}
			else
			{
				Item.seek.iPosition = in_iPosition;
			}
			Item.seek.bIsSeekRelativeToDuration = in_bRelative;
			Item.seek.bSnapToMarker = in_bSeekToNearestMarker;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_Seek>() );
		}

		AKRESULT SeekOnEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_bSeekToNearestMarker ) { return SeekOnEventInternal( in_eventID, in_gameObjectID, in_iPosition, 0.f, false, in_bSeekToNearestMarker ); }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker ) { return SeekOnEventInternal( in_eventID, in_gameObjectID, 0, in_fPercent, true, in_bSeekToNearestMarker ); }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }

		void StopAll( AkGameObjectID in_gameObjectID )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_StopAll;
			Item.stopAll.GameObjID = in_gameObjectID;
			Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_StopAll>() );
		}

		void StopPlayingID( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_StopPlayingID;
			Item.stopEvent.playingID = in_playingID;
			Item.stopEvent.uTransitionDuration = in_uTransitionDuration;
			Item.stopEvent.eFadeCurve = in_eFadeCurve;
			Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_StopPlayingID>() );
		}

		void CancelEventCallbackCookie( void * in_pCookie )
		{
			if( g_pPlayingMgr )
			{
				g_pPlayingMgr->CancelCallbackCookie( in_pCookie );
			}
		}

		void CancelEventCallback( AkPlayingID in_playingID )
		{
			if( g_pPlayingMgr )
			{
				g_pPlayingMgr->CancelCallback( in_playingID );
			}
		}

		AKRESULT GetSourcePlayPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPosition, bool in_bExtrapolate )
		{
			if( !out_puPosition )
			{
				return AK_InvalidParameter;
			}
			return g_pPositionRepository->GetCurrPosition( in_PlayingID, out_puPosition, in_bExtrapolate );
		}

		// -- environments / obstruction ------------------------------------------------------------

		AKRESULT SetEffect( AkUniqueID, AkUInt32, AkUniqueID ) { return AK_Success; }
		/** 2012 0x9174f0 (IH3 fork R). */
		AKRESULT SetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID, const AkSpeakerVolumes & in_sVolumes )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_EnvVolume;
			Item.envvolume.EnvID = in_FXParameterSetID;
			Item.envvolume.sVolumes = in_sVolumes;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_EnvVolume>() );
		}

		/** 2012 0x918960 (IH3 fork R): the same volume on every speaker. */
		AKRESULT SetEnvironmentVolume( AkAuxBusID in_FXParameterSetID, AkReal32 in_fVolume )
		{
			AkSpeakerVolumes volumes;
			volumes.fFrontLeft = volumes.fFrontRight = volumes.fCenter = volumes.fRearLeft = volumes.fRearRight = volumes.fLfe = in_fVolume;
			return SetEnvironmentVolumes( in_FXParameterSetID, volumes );
		}

		/** 2012 0x917550 (IH3 fork R). */
		AKRESULT BypassEnvironment( AkAuxBusID in_FXParameterSetID, bool in_bIsBypassed )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_EnvBypass;
			Item.envbypass.EnvID = in_FXParameterSetID;
			Item.envbypass.bBypass = in_bIsBypassed;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_EnvBypass>() );
		}

		AKRESULT SetGameObjectEnvironmentsValues( AkGameObjectID in_gameObjectID, AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues )
		{
			if( in_uNumEnvValues > 4 )
			{
				return AK_InvalidParameter;
			}
			for( AkUInt32 i = 0; i + 1 < in_uNumEnvValues; ++i )
			{
				for( AkUInt32 j = i + 1; j < in_uNumEnvValues; ++j )
				{
					if( in_aEnvironmentValues[ i ].EnvID == in_aEnvironmentValues[ j ].EnvID )
					{
						return AK_InvalidParameter;
					}
				}
			}
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjEnvValues;
			Item.gameobjenvvalues.GameObjID = in_gameObjectID;
			Item.gameobjenvvalues.uNumValues = in_uNumEnvValues;
			memcpy( Item.gameobjenvvalues.EnvValues, in_aEnvironmentValues, in_uNumEnvValues * sizeof( AkEnvironmentValue ) );
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjEnvValues>() );
		}

		AKRESULT SetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 in_fControlValue )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjDryLevel;
			Item.gameobjdrylevel.GameObjID = in_gameObjectID;
			Item.gameobjdrylevel.fValue = in_fControlValue;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjDryLevel>() );
		}

		AKRESULT SetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32 in_uListener, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_GameObjObstruction;
			Item.gameobjobstr.GameObjID = in_ObjectID;
			Item.gameobjobstr.uListenerIndex = in_uListener;
			Item.gameobjobstr.fObstructionLevel = in_fObstructionLevel;
			Item.gameobjobstr.fOcclusionLevel = in_fOcclusionLevel;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_GameObjObstruction>() );
		}

		// -- global settings -----------------------------------------------------------------------

		AkChannelMask GetSpeakerConfiguration()
		{
			return g_pAkSink ? g_pAkSink->m_SpeakersConfig : 0;
		}

		AkPanningRule GetPanningRule()
		{
			return CAkLEngine::m_panningRule;
		}

		AKRESULT SetPanningRule( AkPanningRule in_ePanningRule )
		{
			AkQueuedMsg Item;
			Item.type = QueuedMsgType_SetPanningRule;
			Item.setPanningRule.panRule = in_ePanningRule;
			return Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_SetPanningRule>() );
		}

		// -- global callbacks (run on the audio thread, once per audio frame) ----------------------

		AKRESULT AddBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			return g_aBehavioralExtensions.AddLast( in_pCallback ) ? AK_Success : AK_InsufficientMemory;
		}

		AKRESULT RemoveBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			return g_aBehavioralExtensions.Remove( in_pCallback );
		}

		AKRESULT RegisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			return AddBehavioralExtension( in_pCallback );
		}

		AKRESULT UnregisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			return g_aBehavioralExtensions.Remove( in_pCallback );
		}

		/** 2012 0x918250 (IH3 fork R): the music engine's hook into every state change. */
		void AddExternalStateHandler( bool ( *in_pHandler )( AkStateGroupID, AkStateID ) )
		{
			g_pExternalStateHandler = in_pHandler;
		}

		// -- Query ---------------------------------------------------------------------------------

		namespace Query
		{
			AKRESULT GetListenerPosition( AkUInt32 in_uIndex, AkListenerPosition & out_rPosition )
			{
				return CAkListener::GetListenerPosition( in_uIndex, out_rPosition );
			}

			AKRESULT GetListenerSpatialization( AkUInt32 in_uIndex, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets )
			{
				return CAkListener::GetListenerSpatialization( in_uIndex, out_rbSpatialized, out_rVolumeOffsets );
			}


			/** 2012 0x91b360 (IH3 fork R): a global or game-object query falls back to the default; the answer always succeeds. */
			AKRESULT GetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				bool bFound = false;
				if( io_rValueType == RTPCValue_Global || io_rValueType == RTPCValue_GameObject )
				{
					CAkRegisteredObj * pGameObj = g_pRegistryMgr->GetObjAndAddref( io_rValueType == RTPCValue_Global ? AK_INVALID_GAME_OBJECT : in_gameObjectID );
					bool bGameObjectSpecific = false;
					bFound = g_pRTPCMgr->GetRTPCValue( in_rtpcID, pGameObj, out_rValue, bGameObjectSpecific );
					io_rValueType = bGameObjectSpecific ? RTPCValue_GameObject : RTPCValue_Global;
					if( pGameObj && --pGameObj->m_refCount <= 0 )
					{
						pGameObj->~CAkRegisteredObj();
						AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
					}
				}
				if( !bFound )
				{
					bool bRTPCFound = false;
					out_rValue = g_pRTPCMgr->GetDefaultValue( in_rtpcID, &bRTPCFound );
					io_rValueType = bRTPCFound ? RTPCValue_Default : RTPCValue_Unavailable;
				}
				return AK_Success;
			}

			/** 2012 0x91b440. */
			AKRESULT GetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				const AkRtpcID id = AkSilent::NameHash( in_pszRtpcName );
				return id ? GetRTPCValue( id, in_gameObjectID, out_rValue, io_rValueType ) : AK_IDNotFound;
			}

			/** 2012 0x91b460. */
			AKRESULT GetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				const AkRtpcID id = AkSilent::NameHash( in_pszRtpcName );
				return id ? GetRTPCValue( id, in_gameObjectID, out_rValue, io_rValueType ) : AK_IDNotFound;
			}

			/** 2012 0x91b480 (IH3 fork R): an unregistered game object is AK_IDNotFound. */
			AKRESULT GetSwitch( AkSwitchGroupID in_switchGroup, AkGameObjectID in_gameObjectID, AkSwitchStateID & out_rSwitchState )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pGameObj = g_pRegistryMgr->GetObjAndAddref( in_gameObjectID );
				if( !pGameObj )
				{
					return AK_IDNotFound;
				}
				out_rSwitchState = g_pRTPCMgr->GetSwitch( in_switchGroup, pGameObj );
				if( --pGameObj->m_refCount <= 0 )
				{
					pGameObj->~CAkRegisteredObj();
					AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
				}
				return AK_Success;
			}

			/** 2012 0x91b500. */
			AKRESULT GetSwitch( const AkOSChar * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				const AkSwitchGroupID id = AkSilent::NameHash( in_pstrSwitchGroupName );
				return id ? GetSwitch( id, in_GameObj, out_rSwitchState ) : AK_IDNotFound;
			}

			/** 2012 0x91b520. */
			AKRESULT GetSwitch( const char * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				const AkSwitchGroupID id = AkSilent::NameHash( in_pstrSwitchGroupName );
				return id ? GetSwitch( id, in_GameObj, out_rSwitchState ) : AK_IDNotFound;
			}

			/** 2012 0x91b050 (IH3 fork R). */
			AKRESULT GetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID, AkSpeakerVolumes & out_rVolumes )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				out_rVolumes = g_pEnvironmentMgr->GetEnvironmentVolumes( in_FXParameterSetID );
				return AK_Success;
			}

			/** 2012 0x91b0b0 (IH3 fork R). */
			AKRESULT GetEnvironmentBypass( AkAuxBusID in_FXParameterSetID, bool & out_rbBypassed )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				out_rbBypassed = g_pEnvironmentMgr->IsBypassed( in_FXParameterSetID );
				return AK_Success;
			}

			/** 2012 0x91b220 (IH3 fork R): the front-left volume. */
			AKRESULT GetEnvironmentVolume( AkAuxBusID in_FXParameterSetID, AkReal32 & out_rVolume )
			{
				AkSpeakerVolumes volumes;
				GetEnvironmentVolumes( in_FXParameterSetID, volumes );
				out_rVolume = volumes.fFrontLeft;
				return AK_Success;
			}

			AkUniqueID GetEventIDFromPlayingID( AkPlayingID in_playingID )
			{
				return g_pPlayingMgr ? g_pPlayingMgr->GetEventIDFromPlayingID( in_playingID ) : AK_INVALID_UNIQUE_ID;
			}

			AkGameObjectID GetGameObjectFromPlayingID( AkPlayingID in_playingID )
			{
				return g_pPlayingMgr ? g_pPlayingMgr->GetGameObjectFromPlayingID( in_playingID ) : AK_INVALID_GAME_OBJECT;
			}

			AKRESULT GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs )
			{
				return g_pPlayingMgr ? g_pPlayingMgr->GetPlayingIDsFromGameObject( in_GameObjId, io_ruNumIDs, out_aPlayingIDs ) : AK_Fail;
			}
		}
	}
}
