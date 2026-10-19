/*=============================================================================
	AkSilentSoundEngine.cpp - AK::SoundEngine's playback side, still on bookkeeping instead of a DSP graph.

	DISHONORED(bringup, then port): agent AN wrote this file as the whole silent engine; agent IH1 (wave 26)
	moved the bank side to the real 2012.1 runtime (src/AkAudioLib/Common: CAkBankMgr, the HIRC object
	hierarchy, CAkRegistryMgr, the Query functions that answer from them, GetIDFromString). What is left
	here is what IH2 replaces with the audio manager / renderer / lower engine.

	It DOES:
	  * initialise and terminate the ported runtime (AkAudioLibInit / AkAudioLibTerm: default pool, index,
	    RTPC / environment / state managers, bank manager thread, registry) from AK::SoundEngine::Init / Term;
	  * hand out monotonic playing ids from PostEvent and remember which game object, event and callback
	    each one belongs to, then fire AK_EndOfEvent on the next RenderAudio / StopAll / StopPlayingID /
	    ExecuteActionOnEvent(Stop) / Term, because USeqAct_AkPostEvent and UDishonoredAudioSystem's
	    end-of-event queue only finish when that callback arrives (agent FI);
	  * mirror every game object call into CAkRegistryMgr (positions, listener mask, environment sends, dry
	    level, scaling, obstruction) and every SetState into CAkStateMgr, so the Query getters answer from the
	    ported managers;
	  * record the last value of every RTPC, switch and trigger and answer the RTPC / switch / listener /
	    playing-id queries from that (IH3 / IH2 own those managers).

	DISHONORED(deviation, agent ID, to be removed by IH2): DishonoredWwise::SetDurationHint keeps one voice alive for a
	given number of wall-clock seconds (the conversation component's estimate of a spoken line, which has no media
	length here), and fires its AK_Duration callback; every voice without a hint still ends on the next RenderAudio.

	It does NOT decode, mix or output audio, and PostEvent does not run an event's actions. Retail returns
	AK_INVALID_PLAYING_ID for an event absent from every loaded bank; here the post still succeeds (Kismet
	latents wait on its end-of-event) and -akdiag logs it, so IH2 has the list before it switches over.
=============================================================================*/
#include <AkBankMgr.h>
#include <AkRegistryMgr.h>
#include "AkSilentInternal.h"

AKRESULT AkAudioLibInit( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings );
void AkAudioLibTerm();
bool AkAudioLibIsEventLoaded( AkUniqueID in_eventID );

#include <chrono>

namespace AkSilent
{
	/** The bank's STMG and the game's SetVolumeThreshold / SetMaxNumVoicesLimit land here (AkAudioLib.cpp). */
	void ApplyVolumeThreshold( AkReal32 in_fVolumeThresholdDB )
	{
		Get().VolumeThreshold = in_fVolumeThresholdDB;
	}

	void ApplyMaxNumVoices( AkUInt16 in_maxNumberVoices )
	{
		Get().MaxNumVoices = in_maxNumberVoices;
	}

	/** Fires AK_EndOfEvent for one playing id and drops it. */
	static void EndPlaying( AkPlayingID id )
	{
		Engine & e = Get();
		std::map<AkPlayingID, Voice>::iterator it = e.Voices.find( id );
		if( it == e.Voices.end() )
		{
			return;
		}
		const Voice entry = it->second;
		e.Voices.erase( it );
		if( entry.Callback && ( entry.Flags & AK_EndOfEvent ) )
		{
			AkEventCallbackInfo info;
			info.pCookie = entry.Cookie;
			info.gameObjID = entry.GameObject;
			info.playingID = entry.ID;
			info.eventID = entry.EventID;
			entry.Callback( AK_EndOfEvent, &info );
		}
	}

	static void EndAllPlaying( AkGameObjectID gameObj )
	{
		Engine & e = Get();
		std::vector<AkPlayingID> doomed;
		for( std::map<AkPlayingID, Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
		{
			if( gameObj == AK_INVALID_GAME_OBJECT || it->second.GameObject == gameObj )
			{
				doomed.push_back( it->first );
			}
		}
		for( size_t i = 0; i < doomed.size(); ++i )
		{
			EndPlaying( doomed[ i ] );
		}
	}

	/**
	 * RenderAudio's end of the silent voices: every voice without a duration hint ends (agent FI), a hinted one only
	 * once its hint has run out on the wall clock between two audio frames (agent ID, until IH2).
	 */
	static void EndElapsedPlaying()
	{
		static std::chrono::steady_clock::time_point s_LastFrame = std::chrono::steady_clock::now();
		const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		const AkReal32 elapsed = std::chrono::duration<AkReal32>( now - s_LastFrame ).count();
		s_LastFrame = now;
		Engine & e = Get();
		std::vector<AkPlayingID> doomed;
		for( std::map<AkPlayingID, Voice>::iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
		{
			if( it->second.HintSecondsLeft > 0.f )
			{
				it->second.HintSecondsLeft -= elapsed;
				if( it->second.HintSecondsLeft > 0.f )
				{
					continue;
				}
				Logf( L"duration hint of playing id %u ran out", it->first );
			}
			doomed.push_back( it->first );
		}
		for( size_t i = 0; i < doomed.size(); ++i )
		{
			EndPlaying( doomed[ i ] );
		}
	}

	static AkPlayingID PostEventCommon( AkUniqueID eventID, AkGameObjectID gameObj, AkUInt32 flags, AkCallbackFunc callback, void * cookie, AkPlayingID requestedID )
	{
		Engine & e = Get();
		if( !e.bSoundEngineInit )
		{
			return AK_INVALID_PLAYING_ID;
		}
		if( !AkAudioLibIsEventLoaded( eventID ) )
		{
			// DISHONORED(port, IH1): retail CAkAudioMgr only posts events present in g_pIndex and returns
			// AK_INVALID_PLAYING_ID otherwise; the silent engine keeps answering (Kismet latents wait on the
			// end-of-event) and counts / logs the difference so IH2 can switch it over with evidence.
			++MutableStats().EventsUnknown;
			const AkOSChar * unknownName = DishonoredWwise::NameForID( eventID );
			AKDIAG( L"akdiag: PostEvent 0x%08x (%s): not in any loaded bank (retail would return AK_INVALID_PLAYING_ID)",
				eventID, unknownName ? unknownName : L"<id only>" );
		}
		Voice entry;
		entry.ID = requestedID ? requestedID : e.NextPlayingID++;
		entry.EventID = eventID;
		entry.GameObject = gameObj;
		entry.Flags = flags;
		entry.Callback = callback;
		entry.Cookie = cookie;
		entry.HintSecondsLeft = 0.f;
		e.Voices[ entry.ID ] = entry;
		++MutableStats().EventsPosted;

		const AkOSChar * name = DishonoredWwise::NameForID( eventID );
		Logf( L"PostEvent %s (id 0x%08x) on game object %u -> playing id %u", name ? name : L"<id only>",
			eventID, (AkUInt32)gameObj, entry.ID );
		return entry.ID;
	}

	AKRESULT AddCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback )
	{
		for( size_t i = 0; i < list.size(); ++i )
		{
			if( list[ i ] == callback )
			{
				return AK_Success;
			}
		}
		list.push_back( callback );
		return AK_Success;
	}

	AKRESULT RemoveCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback )
	{
		for( size_t i = 0; i < list.size(); ++i )
		{
			if( list[ i ] == callback )
			{
				list.erase( list.begin() + i );
				return AK_Success;
			}
		}
		return AK_Fail;
	}

	static String Widen( const char * narrow )
	{
		String wide;
		if( narrow )
		{
			for( const char * p = narrow; *p; ++p )
			{
				wide.push_back( (wchar_t)(unsigned char)*p );
			}
		}
		return wide;
	}
}

namespace DishonoredWwise
{
	void FlushEndOfEventCallbacks()
	{
		AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
	}

	bool SetDurationHint( AkPlayingID in_playingID, AkReal32 in_fSeconds )
	{
		AkSilent::Engine & e = AkSilent::Get();
		std::map<AkPlayingID, AkSilent::Voice>::iterator it = e.Voices.find( in_playingID );
		if( it == e.Voices.end() )
		{
			return false;
		}
		it->second.HintSecondsLeft = in_fSeconds;
		AkSilent::Logf( L"duration hint %.2f s on playing id %u", in_fSeconds, in_playingID );
		if( it->second.Callback && ( it->second.Flags & AK_Duration ) )
		{
			AkDurationCallbackInfo info;
			info.pCookie = it->second.Cookie;
			info.gameObjID = it->second.GameObject;
			info.playingID = it->second.ID;
			info.eventID = it->second.EventID;
			info.fDuration = in_fSeconds * 1000.f;
			info.fEstimatedDuration = in_fSeconds * 1000.f;
			info.audioNodeID = AK_INVALID_UNIQUE_ID;
			it->second.Callback( AK_Duration, &info );
		}
		return true;
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
			if( in_pPlatformSettings )
			{
				e.SpeakerConfig = in_pPlatformSettings->uChannelMask ? in_pPlatformSettings->uChannelMask : AK_SPEAKER_SETUP_5POINT1;
			}
			AkSilent::Logf( L"SoundEngine::Init: silent backend (no output device), command queue %u, speaker mask 0x%x",
				in_pSettings ? in_pSettings->uCommandQueueSize : 0, e.SpeakerConfig );
			return AK_Success;
		}

		void Term()
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return;
			}
			AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
			const DishonoredWwise::Stats & stats = DishonoredWwise::GetStats();
			AkSilent::Logf( L"SoundEngine::Term: %u banks loaded, %u prepared, %u missing, %u events posted, %u RTPC / %u switch / %u state sets",
				stats.BanksLoaded, stats.BanksPrepared, stats.BanksMissing, stats.EventsPosted,
				stats.RtpcSets, stats.SwitchSets, stats.StateSets );
			e.Banks.clear();
			e.GameObjects.clear();
			e.bSoundEngineInit = false;
			AkAudioLibTerm();
		}

		AKRESULT RenderAudio()
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return AK_Fail;
			}
			// The behavioural extensions and global callbacks are what retail's DisAkGlobalCallbackFunc
			// rides on, so they run once per RenderAudio like the real engine's audio frame.
			for( size_t i = 0; i < e.BehaviouralExtensions.size(); ++i )
			{
				e.BehaviouralExtensions[ i ]( false );
			}
			for( size_t i = 0; i < e.GlobalCallbacks.size(); ++i )
			{
				e.GlobalCallbacks[ i ]( false );
			}
			// DISHONORED(bringup, agent FI): a silent event has no duration, so every voice posted since the
			// last audio frame ends on this one. Without it AK_EndOfEvent never arrives and anything latent
			// on a sound waits for ever: USeqAct_AkPostEvent::UpdateOp returns Signal == 0 and Signal is only
			// decremented by AkCallback (akaudioclasses.cpp:48), so l_tower_p's Kismet stopped at op 25 of
			// 1670 on SeqAct_AkPostEvent_0 and the mission's opening sequence was never entered (agent AT).
			// The deviation from retail is only in the WAIT: retail resumes when the sound really finishes,
			// we resume on the next audio frame. Nothing here plays a sound either way.
			// DISHONORED(deviation, agent ID): voices given a DishonoredWwise::SetDurationHint are kept until it runs out.
			AkSilent::EndElapsedPlaying();
			return AK_Success;
		}

		// -- plug-ins ------------------------------------------------------------------------------

		AKRESULT RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback, AkCreateParamCallback )
		{
			// DISHONORED(bringup): recorded, not instantiated. Retail registers eight effect plug-ins and
			// one source plug-in here (UAkAudioDevice::EnsureInitialized, 2013 rva 0x5b1430); without a
			// DSP graph there is nothing for them to process.
			AkSilent::Logf( L"SoundEngine::RegisterPlugin: type %d, company %u, plug-in %u (recorded, no DSP graph)",
				(int)in_eType, in_ulCompanyID, in_ulPluginID );
			return AK_Success;
		}

		AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback, AkCreateBankSourceCallback )
		{
			AkSilent::Logf( L"SoundEngine::RegisterCodec: company %u, codec %u (recorded, no media decoding)", in_ulCompanyID, in_ulCodecID );
			return AK_Success;
		}

		// -- game objects --------------------------------------------------------------------------

		AKRESULT RegisterGameObj( AkGameObjectID in_gameObjectID, const char * in_pszObjName )
		{
			if( in_gameObjectID == AK_INVALID_GAME_OBJECT )
			{
				return AK_InvalidParameter;
			}
			AkSilent::GameObject obj;
			obj.Name = AkSilent::Widen( in_pszObjName );
			memset( &obj.Position, 0, sizeof( obj.Position ) );
			obj.ListenerMask = 0xFFFFFFFF;
			obj.AttenuationScale = 1.f;
			obj.ObstructionLevel = 0.f;
			obj.OcclusionLevel = 0.f;
			obj.DryLevel = 1.f;
			AkSilent::Get().GameObjects[ in_gameObjectID ] = obj;
			AkAutoLock<CAkLock> gate( g_csMain );
			return g_pRegistryMgr && g_pRegistryMgr->RegisterObject( in_gameObjectID, NULL ) ? AK_Success : AK_Fail;
		}

		AKRESULT UnregisterGameObj( AkGameObjectID in_gameObjectID )
		{
			AkSilent::EndAllPlaying( in_gameObjectID );
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				if( g_pRegistryMgr )
				{
					g_pRegistryMgr->UnregisterObject( in_gameObjectID );
				}
			}
			return AkSilent::Get().GameObjects.erase( in_gameObjectID ) ? AK_Success : AK_Fail;
		}

		AKRESULT UnregisterAllGameObj()
		{
			AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
			AkSilent::Get().GameObjects.clear();
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pRegistryMgr )
			{
				g_pRegistryMgr->UnregisterAll();
			}
			return AK_Success;
		}

		// -- positioning ---------------------------------------------------------------------------

		AKRESULT SetPosition( AkGameObjectID in_gameObjectID, const AkSoundPosition & in_Position, AkUInt32 in_ulListenerIndex )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.Position = in_Position;
			AkAutoLock<CAkLock> gate( g_csMain );
			return g_pRegistryMgr ? g_pRegistryMgr->SetPosition( in_gameObjectID, &in_Position, 1, MultiPositionType_SingleSource, in_ulListenerIndex ) : AK_Fail;
		}

		AKRESULT SetMultiplePositions( AkGameObjectID in_gameObjectID, const AkSoundPosition * in_pPositions, AkUInt16 in_NumPositions, MultiPositionType in_eMultiPositionType )
		{
			if( !in_pPositions || !in_NumPositions )
			{
				return AK_InvalidParameter;
			}
			const AKRESULT eResult = SetPosition( in_gameObjectID, in_pPositions[ 0 ] );
			AkAutoLock<CAkLock> gate( g_csMain );
			if( eResult == AK_Success && g_pRegistryMgr )
			{
				g_pRegistryMgr->SetPosition( in_gameObjectID, in_pPositions, in_NumPositions, in_eMultiPositionType, 31 );
			}
			return eResult;
		}

		AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex )
		{
			if( in_uIndex >= 8 )
			{
				return AK_InvalidParameter;
			}
			AkSilent::Get().Listeners[ in_uIndex ] = in_Position;
			return AK_Success;
		}

		AKRESULT SetActiveListeners( AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.ListenerMask = in_uListenerMask;
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pRegistryMgr )
			{
				g_pRegistryMgr->SetActiveListeners( in_gameObjectID, in_uListenerMask );
			}
			return AK_Success;
		}

		AKRESULT SetAttenuationScalingFactor( AkGameObjectID in_gameObjectID, AkReal32 in_fAttenuationScalingFactor )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.AttenuationScale = in_fAttenuationScalingFactor;
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pRegistryMgr )
			{
				g_pRegistryMgr->SetGameObjectScalingFactor( in_gameObjectID, in_fAttenuationScalingFactor );
			}
			return AK_Success;
		}

		AKRESULT SetListenerScalingFactor( AkUInt32, AkReal32 ) { return AK_Success; }
		AKRESULT SetListenerSpatialization( AkUInt32, bool, AkSpeakerVolumes * ) { return AK_Success; }
		AKRESULT SetListenerPipeline( AkUInt32, bool, bool ) { return AK_Success; }

		// -- game syncs ----------------------------------------------------------------------------

		AKRESULT SetRTPCValue( AkRtpcID in_rtpcID, AkRtpcValue in_value, AkGameObjectID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::Get().GlobalRtpc[ in_rtpcID ] = in_value;
			++AkSilent::MutableStats().RtpcSets;
			return AK_Success;
		}

		AKRESULT SetRTPCValue( const AkOSChar * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return SetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT SetRTPCValue( const char * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return SetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT ResetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::Get().GlobalRtpc.erase( in_rtpcID );
			return AK_Success;
		}

		AKRESULT ResetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return ResetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT ResetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return ResetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT SetSwitch( AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			e.Switches[ in_switchGroup ] = in_switchState;
			if( in_gameObjectID != AK_INVALID_GAME_OBJECT )
			{
				e.ObjectSwitches[ ( (AkUInt64)in_gameObjectID << 32 ) | in_switchGroup ] = in_switchState;
			}
			++AkSilent::MutableStats().SwitchSets;
			return AK_Success;
		}

		AKRESULT SetSwitch( const AkOSChar * in_pszSwitchGroup, const AkOSChar * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			const AkSwitchGroupID group = AkSilent::RegisterName( in_pszSwitchGroup );
			const AkSwitchStateID state = AkSilent::RegisterName( in_pszSwitchState );
			AkSilent::Logf( L"SetSwitch %s = %s on game object %u", in_pszSwitchGroup, in_pszSwitchState, (AkUInt32)in_gameObjectID );
			return SetSwitch( group, state, in_gameObjectID );
		}

		AKRESULT SetSwitch( const char * in_pszSwitchGroup, const char * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			return SetSwitch( AkSilent::RegisterName( in_pszSwitchGroup ), AkSilent::RegisterName( in_pszSwitchState ), in_gameObjectID );
		}

		AKRESULT SetState( AkStateGroupID in_stateGroup, AkStateID in_state )
		{
			AkSilent::Get().States[ in_stateGroup ] = in_state;
			++AkSilent::MutableStats().StateSets;
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pStateMgr )
			{
				AkStateGroupInfo * pGroup = g_pStateMgr->AddStateGroup( in_stateGroup );
				if( pGroup )
				{
					pGroup->ActualState = in_state;
				}
			}
			return AK_Success;
		}

		AKRESULT SetState( const AkOSChar * in_pszStateGroup, const AkOSChar * in_pszState )
		{
			const AkStateGroupID group = AkSilent::RegisterName( in_pszStateGroup );
			const AkStateID state = AkSilent::RegisterName( in_pszState );
			AkSilent::Logf( L"SetState %s = %s", in_pszStateGroup, in_pszState );
			return SetState( group, state );
		}

		AKRESULT SetState( const char * in_pszStateGroup, const char * in_pszState )
		{
			return SetState( AkSilent::RegisterName( in_pszStateGroup ), AkSilent::RegisterName( in_pszState ) );
		}

		AKRESULT PostTrigger( AkTriggerID, AkGameObjectID )
		{
			++AkSilent::MutableStats().TriggersPosted;
			return AK_Success;
		}

		AKRESULT PostTrigger( const AkOSChar * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			AkSilent::Logf( L"PostTrigger %s on game object %u", in_pszTrigger, (AkUInt32)in_gameObjectID );
			return PostTrigger( AkSilent::RegisterName( in_pszTrigger ), in_gameObjectID );
		}

		AKRESULT PostTrigger( const char * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			return PostTrigger( AkSilent::RegisterName( in_pszTrigger ), in_gameObjectID );
		}

		// -- events --------------------------------------------------------------------------------

		AkPlayingID PostEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32, AkExternalSourceInfo *, AkPlayingID in_PlayingID )
		{
			return AkSilent::PostEventCommon( in_eventID, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_PlayingID );
		}

		AkPlayingID PostEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		AkPlayingID PostEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		AKRESULT ExecuteActionOnEvent( AkUniqueID in_eventID, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs, AkCurveInterpolation, AkPlayingID in_PlayingID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( in_ActionType != AkActionOnEventType_Stop && in_ActionType != AkActionOnEventType_Break )
			{
				return AK_Success;			// pause / resume have nothing to act on without voices
			}
			std::vector<AkPlayingID> doomed;
			for( std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
			{
				const bool bMatchesEvent = ( in_eventID == AK_INVALID_UNIQUE_ID ) || ( it->second.EventID == in_eventID );
				const bool bMatchesObject = ( in_gameObjectID == AK_INVALID_GAME_OBJECT ) || ( it->second.GameObject == in_gameObjectID );
				const bool bMatchesPlaying = ( in_PlayingID == AK_INVALID_PLAYING_ID ) || ( it->first == in_PlayingID );
				if( bMatchesEvent && bMatchesObject && bMatchesPlaying )
				{
					doomed.push_back( it->first );
				}
			}
			for( size_t i = 0; i < doomed.size(); ++i )
			{
				AkSilent::EndPlaying( doomed[ i ] );
			}
			return AK_Success;
		}

		AKRESULT ExecuteActionOnEvent( const AkOSChar * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		AKRESULT ExecuteActionOnEvent( const char * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		void StopAll( AkGameObjectID in_gameObjectID )
		{
			AkSilent::EndAllPlaying( in_gameObjectID );
		}

		void StopPlayingID( AkPlayingID in_playingID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::EndPlaying( in_playingID );
		}

		AKRESULT SeekOnEvent( AkUniqueID, AkGameObjectID, AkTimeMs, bool ) { return AK_Success; }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( AkUniqueID, AkGameObjectID, AkReal32, bool ) { return AK_Success; }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }

		void CancelEventCallbackCookie( void * in_pCookie )
		{
			AkSilent::Engine & e = AkSilent::Get();
			for( std::map<AkPlayingID, AkSilent::Voice>::iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
			{
				if( it->second.Cookie == in_pCookie )
				{
					it->second.Callback = NULL;
				}
			}
		}

		void CancelEventCallback( AkPlayingID in_playingID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			std::map<AkPlayingID, AkSilent::Voice>::iterator it = e.Voices.find( in_playingID );
			if( it != e.Voices.end() )
			{
				it->second.Callback = NULL;
			}
		}

		AKRESULT GetSourcePlayPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPosition, bool )
		{
			if( !out_puPosition )
			{
				return AK_InvalidParameter;
			}
			*out_puPosition = 0;
			return AkSilent::Get().Voices.count( in_PlayingID ) ? AK_Success : AK_Fail;
		}

		// -- environments / obstruction ------------------------------------------------------------

		AKRESULT SetEffect( AkUniqueID, AkUInt32, AkUniqueID ) { return AK_Success; }
		AKRESULT SetEnvironmentVolume( AkAuxBusID, AkReal32 ) { return AK_Success; }
		AKRESULT SetEnvironmentVolumes( AkAuxBusID, const AkSpeakerVolumes & ) { return AK_Success; }
		AKRESULT BypassEnvironment( AkAuxBusID, bool ) { return AK_Success; }
		AKRESULT SetGameObjectEnvironmentsValues( AkGameObjectID in_gameObjectID, AkEnvironmentValue * in_aEnvironmentValues, AkUInt32 in_uNumEnvValues )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			return g_pRegistryMgr ? g_pRegistryMgr->SetGameObjectEnvironmentsValues( in_gameObjectID, in_aEnvironmentValues, in_uNumEnvValues ) : AK_Fail;
		}

		AKRESULT SetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 in_fControlValue )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.DryLevel = in_fControlValue;
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pRegistryMgr )
			{
				g_pRegistryMgr->SetGameObjectDryLevelValue( in_gameObjectID, in_fControlValue );
			}
			return AK_Success;
		}

		AKRESULT SetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32 in_uListener, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_ObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.ObstructionLevel = in_fObstructionLevel;
			it->second.OcclusionLevel = in_fOcclusionLevel;
			AkAutoLock<CAkLock> gate( g_csMain );
			if( g_pRegistryMgr )
			{
				g_pRegistryMgr->SetObjectObstructionAndOcclusion( in_ObjectID, in_uListener, in_fObstructionLevel, in_fOcclusionLevel );
			}
			return AK_Success;
		}

		// -- global settings -----------------------------------------------------------------------

		AkChannelMask GetSpeakerConfiguration() { return AkSilent::Get().SpeakerConfig; }
		AkPanningRule GetPanningRule() { return AkSilent::Get().PanningRule; }

		AKRESULT SetPanningRule( AkPanningRule in_ePanningRule )
		{
			AkSilent::Get().PanningRule = in_ePanningRule;
			return AK_Success;
		}

		// -- global callbacks ----------------------------------------------------------------------

		AKRESULT RegisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			if( !in_pCallback )
			{
				return AK_InvalidParameter;
			}
			return AkSilent::AddCallback( AkSilent::Get().GlobalCallbacks, in_pCallback );
		}

		AKRESULT UnregisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			return AkSilent::RemoveCallback( AkSilent::Get().GlobalCallbacks, in_pCallback );
		}

		AKRESULT AddBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			if( !in_pCallback )
			{
				return AK_InvalidParameter;
			}
			return AkSilent::AddCallback( AkSilent::Get().BehaviouralExtensions, in_pCallback );
		}

		AKRESULT RemoveBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			return AkSilent::RemoveCallback( AkSilent::Get().BehaviouralExtensions, in_pCallback );
		}

		void AddExternalStateHandler( bool ( * )( AkStateGroupID, AkStateID ) ) {}

		// -- Query ---------------------------------------------------------------------------------

		namespace Query
		{
			AKRESULT GetListenerPosition( AkUInt32 in_uIndex, AkListenerPosition & out_rPosition )
			{
				if( in_uIndex >= 8 )
				{
					return AK_InvalidParameter;
				}
				out_rPosition = AkSilent::Get().Listeners[ in_uIndex ];
				return AK_Success;
			}

			AKRESULT GetListenerSpatialization( AkUInt32, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets )
			{
				out_rbSpatialized = true;
				memset( &out_rVolumeOffsets, 0, sizeof( out_rVolumeOffsets ) );
				return AK_Success;
			}

			// DISHONORED(bringup): an attenuation radius is bank logic; without it every object reports -1,
			// which is exactly what UAkEvent::GetMaxRadius stores for "no attenuation".
			AkReal32 GetMaxRadius( AkGameObjectID ) { return -1.f; }

			AKRESULT GetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				std::map<AkRtpcID, AkRtpcValue> & rtpc = AkSilent::Get().GlobalRtpc;
				std::map<AkRtpcID, AkRtpcValue>::const_iterator it = rtpc.find( in_rtpcID );
				if( it == rtpc.end() )
				{
					out_rValue = 0.f;
					io_rValueType = RTPCValue_Unavailable;
					return AK_Fail;
				}
				out_rValue = it->second;
				io_rValueType = RTPCValue_Global;
				return AK_Success;
			}

			AKRESULT GetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				return GetRTPCValue( AkSilent::NameHash( in_pszRtpcName ), in_gameObjectID, out_rValue, io_rValueType );
			}

			AKRESULT GetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				return GetRTPCValue( AkSilent::NameHash( in_pszRtpcName ), in_gameObjectID, out_rValue, io_rValueType );
			}

			AKRESULT GetSwitch( AkSwitchGroupID in_switchGroup, AkGameObjectID in_gameObjectID, AkSwitchStateID & out_rSwitchState )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkUInt64, AkSwitchStateID>::const_iterator perObject = e.ObjectSwitches.find( ( (AkUInt64)in_gameObjectID << 32 ) | in_switchGroup );
				if( perObject != e.ObjectSwitches.end() )
				{
					out_rSwitchState = perObject->second;
					return AK_Success;
				}
				std::map<AkSwitchGroupID, AkSwitchStateID>::const_iterator global = e.Switches.find( in_switchGroup );
				if( global == e.Switches.end() )
				{
					out_rSwitchState = AK_DEFAULT_SWITCH_STATE;
					return AK_Fail;
				}
				out_rSwitchState = global->second;
				return AK_Success;
			}

			AKRESULT GetSwitch( const AkOSChar * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				return GetSwitch( AkSilent::NameHash( in_pstrSwitchGroupName ), in_GameObj, out_rSwitchState );
			}

			AKRESULT GetSwitch( const char * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				return GetSwitch( AkSilent::NameHash( in_pstrSwitchGroupName ), in_GameObj, out_rSwitchState );
			}

			AKRESULT GetEnvironmentVolumes( AkAuxBusID, AkSpeakerVolumes & out_rVolumes )
			{
				memset( &out_rVolumes, 0, sizeof( out_rVolumes ) );
				return AK_Success;
			}

			AKRESULT GetEnvironmentBypass( AkAuxBusID, bool & out_rbBypassed )
			{
				out_rbBypassed = false;
				return AK_Success;
			}

			AKRESULT GetEnvironmentVolume( AkAuxBusID, AkReal32 & out_rVolume )
			{
				out_rVolume = 0.f;
				return AK_Success;
			}

			AkUniqueID GetEventIDFromPlayingID( AkPlayingID in_playingID )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.find( in_playingID );
				return it == e.Voices.end() ? AK_INVALID_UNIQUE_ID : it->second.EventID;
			}

			AkGameObjectID GetGameObjectFromPlayingID( AkPlayingID in_playingID )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.find( in_playingID );
				return it == e.Voices.end() ? AK_INVALID_GAME_OBJECT : it->second.GameObject;
			}

			AKRESULT GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs )
			{
				AkSilent::Engine & e = AkSilent::Get();
				AkUInt32 written = 0;
				for( std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
				{
					if( it->second.GameObject != in_GameObjId )
					{
						continue;
					}
					if( out_aPlayingIDs && written < io_ruNumIDs )
					{
						out_aPlayingIDs[ written ] = it->first;
					}
					++written;
				}
				io_ruNumIDs = written;
				return AK_Success;
			}
		}
	}
}
